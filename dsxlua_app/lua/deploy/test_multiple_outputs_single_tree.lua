--[[

root --- SDI out A --- File A
	  |				|
	  -- SDI out B --

]]

-- get the dsx module
local dsx = require'hapi.dsx'
local critical_section = dsx.new_cs()
local g_status_pos = -1
local trim = {1001, 1300}

local my_pl	= {
	resolution = "480i_30M_4x3",
	elements = {
		{
			name = "E:\\Beavis and Butthead.mpg",
			trim_in = trim[1],
			trim_out = trim[2]
		},
	}
}

print=dsx.output_debug

-- create the board
print">creating board"
dsx.boards[1]{profile="480i_30M_4x3"}

-- create a pump
print">creating pump"
dsx.pump[1]{}

-- create a file reader
print">creating file reader"
local file_reader = dsx.boards[1].file_reader{profile="480i_30M_4x3", pump=dsx.pump[1]}

-- create a live window
print">creating live window"
local live_window = dsx.boards[1].live_window{profile="480i_30M_4x3", create_internal_window=true, pump=dsx.pump[1]}

-- create the hardware filters
print">creating dma out av"
local dma_out_av_1 = dsx.boards[1].dma_out_av{profile="480i_30M_4x3", pump=dsx.pump[1]}
local dma_out_av_2 = dsx.boards[1].dma_out_av{profile="480i_30M_4x3", pump=dsx.pump[1]}

print">creating output"
local out_1 = dsx.boards[1].output{profile="480i_30M_4x3", pump=dsx.pump[1]}
local out_2 = dsx.boards[1].output{profile="480i_30M_4x3", pump=dsx.pump[1]}

print">creating root"
local root = dsx.boards[1].root{profile="480i_30M_4x3", pump=dsx.pump[1]}

print">creating normalizer"
local normalizer = dsx.boards[1].normalizer{profile="480i_30M_4x3", pump=dsx.pump[1]}

print">creating audio level"
local audio_level = dsx.boards[1].audio_level{profile="480i_30M_4x3", pump=dsx.pump[1]}

print">connecting filters"

-- connect the filters

--[[
root <---- live window <------------------ normalizer <----file_reader
		|										|
		-- output 0 <---- dma_out_av 0 ---<----audio_level
		|								  |
		-- output 1 <---- dma_out_av 1 ---
		
]]
root{{{add_source=live_window}}}
root{{{add_source=dsx.boards.out_1}}}
root{{{add_source=out_2}}}
live_window{{{add_source=normalizer}}}
out_1{{{add_source=dma_out_av_1}}}
out_2{{{add_source=dma_out_av_2}}}
dma_out_av_1{{{add_source=audio_level}}}
dma_out_av_2{{{add_source=audio_level}}}
audio_level{{{add_source=normalizer}}}
normalizer{{{add_source=file_reader_1}}}

-- cue the file reader. Note: replace file with a valid filename
print">cueing reader"
file_reader{[dsx.boards[1].clock.now]={{cue={playlist=my_pl, start_pos=1, iterator={pos=1}}}}}

-- start the pump, this will start the playback. Note that we start the pump from the filter at the root of the hierarchy.
print">starting the pump"
root{{{pump={presentation_time=dsx.boards[1].clock.now, pace=1}}}}

-- play until the end of the file is reached
critical_section.lock=true
while g_status_pos<trim[2] do
	print(">status pos: "..g_status_pos)
	critical_section.lock=false

	-- don't query status too aggressively, good practice... I chose to wait one second between each call
	dsx.boards[1].clock{[dsx.boards[1].clock.now]={{wait_until_nanotime=dsx.boards[1].clock.now+1*1000*1000*10}}}
	critical_section.lock=true
end
critical_section.lock=false

-- we're done! stop the pump
print">stopping the pump"
root{{{pump=0}}}

print">disconnecting filters"

-- disconnect the filters
root{{{remove_source="all"}}}
live_window{{{remove_source="all"}}}
output_1{{{remove_source="all"}}}
output_2{{{remove_source="all"}}}
dma_out_av_1{{{remove_source="all"}}}
dma_out_av_2{{{remove_source="all"}}}
audio_level{{{remove_source="all"}}}
normalizer{{{remove_source="all"}}}

-- that's it!