-- get the dsx module
local dsx = require'hapi.dsx'
local output, dma_out_av, file_reader -- filters
local pump
local sync_state = {}
local clock

print=dsx.output_debug

-- create the board
print">creating board"
dsx.boards[1]{profile="480i_30M_4x3"}

-- helper
clock = dsx.boards[1].clock

-- create a pump
print">creating pump"
pump = dsx.pump()

-- create a file reader
print">creating file reader"
file_reader=dsx.boards[1].file_reader{pump=pump}

-- create the hardware filters

print">creating dma out av"
dma_out_av=dsx.boards[1].dma_out_av{pump=pump}

print">creating output"
output=dsx.boards[1].output{profile="480i_30M_4x3", channel=1, buffering_in_frames=8, audio={"aes_1"}, pump=pump}

print">connecting filters"

-- connect the filters
--[[
--		output <--- dma_out_av <--- file_reader
--]]

output{{source=dma_out_av}}
dma_out_av{{source=file_reader}}

-- cue the file reader. Note: replace file with a valid filename
playlist = {
	id = 1,							-- unique user-defined id number
	resolution = "480i_30M_4x3",	-- Matrox requires a base resolution for each playlist
	elements = {
		{
			name = "C:\\12402Unity Care short filer 1.06.mov",
			trim_in = 1,
			trim_out = 300
		},
	}
}

-- a complete file reader state
local reader_state = {
	pump				= pump,
	playlist			= playlist,
	position			= 1,
	position_increment	= 1,
	tdir				= false,
	mode				= "play",
	audio_scrub			= false,
	iterator			= fn_iterator
}

-- {tv/synchronous={{args}}}
print">cueing reader"
file_reader{reader_state}

-- start the pump, this will start the playback. Note that we start the pump from the filter at the root of the hierarchy.
-- throttle at 1.0, software throttle based on the clock ticks
print">starting the pump"
output{{pump={presentation_time=clock.now}}}

-- play until the end of the file is reached
print">querying status and waiting"
repeat
	-- don't query status too aggressively, good practice... I chose to wait one second between each call
	clock{{state={wait_until_nanotime=clock.now+1*1000*1000*10}}}

	file_reader{[clock.now]=sync_state}}
	print(">status pos: "..sync_state.position)	
until sync_state.position < 300

-- we're done! stop the pump
print">stopping the pump"
output{{pump=false}}

print">disconnecting filters"

-- disconnect the filters
output{{source=false}}
dma_out_av{{source=false}}

-- that's it!