--[[

root <-- file_writer <-- dma_av_in <-- input_A
	  |				  |
	  -- live_window <-
]]

-- get the dsx module
local dsx							= require'hapi.dsx'
local file_av						= "e:\\test_single_encode.mpg"
local profile						= "480i_30M_4x3"
local root, lw, fw, dma_in, input -- filters
local cs							= dsx.new_cs()
local g_status						= -1
local one_second_nanotime			= 10*1000*1000

print=dsx.output_debug

-- create the board
print">creating board"
dsx.boards[1]{profile=profile}

local clock = dsx.boards[1].clock

-- create a pump
print">creating pump"
dsx.pump[1]{}

-- create a root
print">creating root filter"
root = dsx.boards[1].root{profile=profile, pump=dsx.pump[1]}

-- create a file writer
print">creating file writer"
fw = dsx.boards[1].file_writer{profile=profile, pump=dsx.pump[1], callbacks={status=
	function (in_status)
		print("callback pos: "..in_status.pos)
			cs.lock=true
			g_status=in_status.pos
			cs.lock=false
	end}}

-- create a live window
--print">creating live window"
--lw = dsx.boards[1].live_window[1]{profile=profile, create_internal_window=true, pump=dsx.pump[1]}

-- create the hardware filters
print">creating dma in av"
dma_in_av = dsx.boards[1].dma_in_av{profile=profile, pump=dsx.pump[1]}

print">creating input"
input = dsx.boards[1].input{
	profile=profile, 
	pump=dsx.pump[1], 
	buffering={frames=8},
	video={"sdi_1"},
	audio={"embedded_1"}}

--print">creating normalizer"
--dsx.boards[1].normalizer[1]{profile="480i_30M_4x3", pump=dsx.pump[1]}

print">connecting filters"

-- connect the filters
--[[
root <-- file_writer <-- dma_av_in <-- input_A
	  |				  |
	  -- live_window <-
]]
root{{{add_source=fw}}}
--root{{{add_source=lw}}}
--lw{{{add_source=dma_in_av}}}
fw{{{add_source=dma_in_av}}}
dma_in_av{{{add_source=input}}}

print">cueing writer"

local time = os.date("*t", os.time())
	
fw{{{cue={
	filename_video=file_av, 
	filename_audio=file_av, 
	bitrate_mbps=5, 
	start_pos=0,
	buffering={frames=30}, -- file writer host pipeline size in video frames
	tc={hour=time.hour, minute=time.min, second=time.sec, frame=0},
	codec_video={encoder="mpeg2", profile="main", level="main", muxing="program"}, 
	codec_audio={encoder="mpeg1layer2"}}}}}

print">turn on file writing, the file writer will write the first buffer it receives"
fw{{{encode=true}}}

-- start the pump using a timestamp marking the first frame to come out of the input
print">starting the pump"
root{{{pump={presentation_time=dsx.boards[1].clock.now + one_second_nanotime}}}}

-- wait for an arbitrary number of frames
cs.lock=true
while g_status<300 do
	print(">>>main script loop pos: "..g_status)
	cs.lock=false

	-- don't query status too aggressively, good practice... I chose to wait one second between each call
	clock{{{wait_until_nanotime=clock.now + one_second_nanotime}}}
	cs.lock=true
end
cs.lock=false

print">stopping the encode"
fw{{{encode=false}}}

-- we're done! stop the pump
print">stopping the pump"
root{{{pump=0}}}

print">disconnecting filters"

-- disconnect the filters
root{{{remove_source="all"}}}
--lw{{{remove_source="all"}}}
fw{{{remove_source="all"}}}
dma_in_av{{{remove_source="all"}}}

-- that's it!