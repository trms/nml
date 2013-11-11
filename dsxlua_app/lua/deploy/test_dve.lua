--[[

root <-- output <-- dma_out_av <-- compositor <-- file_reader_A
	  |				  |						   |	
	  -- live_window <-						   -- dve <-- file_reader_B
]]

-- get the dsx module
local dsx = require'hapi.dsx'
local root, output, live_window, dma_out_av, compositor, file_reader_A, file_reader_B, dve -- filters
local clock
local profile="480i_30M_4x3"
local width=720		-- frame size, used in the compositor keyframes
local height=486
local band=10
local test_duration_in_frames = 3000
local one_second_nanos = 1*1000*1000*10
local dve_size={width/2, height/2}
local playlist_A = {
	resolution = profile,
	elements = {
		{
			name = "e:\\Beavis and Butthead.mpg",
			trim_in = 1,
			trim_out = test_duration_in_frames
		},
	}
}
local playlist_B = {
	resolution = profile,
	elements = {
		{
			name = "e:\\16527Santa Clara County Housing, Land Use, Environment And Transportation Committee Meeting 130120 (130116) cue 3.08- 26.29.mpg",
			trim_in = 1,
			trim_out = test_duration_in_frames
		},
	}
}

print=dsx.output_debug

-- create the board
print(">creating board #"..dsx.boards[1].serial_number)
dsx.boards[1]{profile=profile}
clock=dsx.boards[1].clock

-- create a pump
print">creating pump"
dsx.pump[1]{}

-- root
print">creating root"
root=dsx.boards[1].root{pump=dsx.pump[1]}

--dve
print">creating dve"
dve=dsx.boards[1].dve{pump=dsx.pump[1]}

-- create a file reader
print">creating file readers"
file_reader_A=dsx.boards[1].file_reader{pump=dsx.pump[1]}
file_reader_B=dsx.boards[1].file_reader{pump=dsx.pump[1]}

-- compositor
print">creating compositor"
compositor=dsx.boards[1].compositor{pump=dsx.pump[1]}

-- live window
print">creating live window"
live_window=dsx.boards[1].live_window{pump=dsx.pump[1], create_internal_window=true}

-- create the hardware filters

print">creating dma out av"
dma_out_av=dsx.boards[1].dma_out_av{pump=dsx.pump[1]}

print">creating output"
output=dsx.boards[1].output{profile=profile, video={"sdi_1"}, buffering={frames=8}, pump=dsx.pump[1]}

print">connecting filters"

-- connect the filters
-- NOTE: this is using a sw scaler window
--[[

root <-- output <-- dma_out_av <-- compositor <-- file_reader_A
	  |				  |						   |	
	  -- live_window <-						   -- dve <-- file_reader_B
]]

root{{{add_source=output}}}
root{{{add_source=live_window}}}
output{{{add_source=dma_out_av}}}
dma_out_av{{{add_source=compositor}}}
live_window{{{add_source=compositor}}}
dve{{{add_source=file_reader_B}}}

-- tell the compositor which layer the sources belong to
-- 1-based except opacity where 0 is transparent
compositor{{{add_source={source=file_reader_A, layer={zindex=0, top=0, left=0, opacity=1.0}}}}}
compositor{{{add_source={source=dve, layer={zindex=1, top=0, left=0, opacity=1.0}}}}}

print">cueing reader A"
file_reader_A{{{cue={playlist=playlist_A, start_pos=1, iterator={pos=1}}}}}

print">cueing reader B"
file_reader_B{{{cue={playlist=playlist_B, start_pos=1, iterator={pos=1}}}}}

-- set the dve keyframe
--[[

	-------------------------
	|			------------|
	|			|		   ||
	|			|		   ||
	|			------------|
	|						|
	|						|
	-------------------------
]]

-- this is one based but whatever
-- NOTE: the dve outputs a full screen 4224 image. The dve source is scaled into the full screen destination buffer using an alpha mask.
-- the compositor will perform the translation
dve{{{layer={scale={width=width/2, height=height/2}, crop={top=0, left=0, bottom=0, right=0}}}}}

-- start the pump, this will start the playback. Note that we start the pump from the filter at the root of the hierarchy.
-- throttle at 1.0, software throttle based on the clock ticks
print">starting the pump"
local pumpStart = clock.now+2*one_second_nanos
root{{{pump={presentation_time=pumpStart, pace=1}}}}

-- wait 10 seconds
local time=clock.now
while clock.now < time + 10*one_second_nanos do
	print">waiting."
	clock{{{wait_until_nanotime=clock.now+one_second_nanos}}}
end

-- done

-- we're done! stop the pump
print">stopping the pump"
root{{{pump=0}}}

print">disconnecting filters"

-- disconnect the filters
root{{{remove_source="all"}}}
output{{{remove_source="all"}}}
live_window{{{remove_source="all"}}}
dma_out_av{{{remove_source="all"}}}
compositor{{{remove_source="all"}}}
dve{{{remove_source="all"}}}

-- that's it!