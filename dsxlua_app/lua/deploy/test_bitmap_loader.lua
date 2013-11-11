--[[

root <-- output <-- dma_out_av <-- compositor <-- file_reader_A
	  |				   			|				|	
	  -- live_window <-----------				-- positioner <--bitmap_reader (crawl)
												|
												-- bitmap_reader (logo)
]]

-- get the dsx module
local dsx = require'hapi.dsx'
local root, output, live_window, dma_out_av, compositor, file_reader_A, bitmap_reader, crawl_reader -- filters
local clock
local profile="480i_30M_4x3"
local width=720		-- frame size, used in the compositor keyframes
local height=486
local band=10
local test_duration_in_frames = 3000
local one_second_nanos = 1*1000*1000*10
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
local bitmap = "E:\\crawls\\NTSC\\logo.png"
local crawl = "E:\\crawls\\NTSC\\crawl.png"

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

-- create a file reader
print">creating file reader"
file_reader_A=dsx.boards[1].file_reader{pump=dsx.pump[1]}

-- the bitmap reader
print">creating bitmap reader"
bitmap_reader=dsx.boards[1].gdi_bitmap_reader{pump=dsx.pump[1]}

-- second bitmap reader
print">creating bitmap reader (crawl)"
crawl_reader=dsx.boards[1].gdi_bitmap_reader{pump=dsx.pump[1]}

-- positioner, works in conjunction with the crawl
print">creating positioner"
positioner=dsx.boards[1].positioner{pump=dsx.pump[1]}

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
	  |				   			|				|	
	  -- live_window <-----------				-- positioner <--bitmap_reader (crawl)
												|
												-- bitmap_reader (logo)
]]

root{{{add_source=output}}}
output{{{add_source=dma_out_av}}}
dma_out_av{{{add_source=compositor}}}

root{{{add_source=live_window}}}
live_window{{{add_source=compositor}}}

-- tell the compositor which layer the sources belong to
-- 1-based except opacity where 0 is transparent
compositor{{{add_source={source=file_reader_A, layer={zindex=0, top=0, left=0, opacity=1.0}}},
			{add_source={source=positioner, layer={zindex=1, top=0, left=0, opacity=1.0}}},
			{add_source={source=bitmap_reader, region={layer=2, top=0, left=0, opacity=1.0}}}}}
positioner{{{add_source=crawl_reader}}}

print">cueing reader A"
file_reader_A{{{cue={playlist=playlist_A, start_pos=1, iterator={pos=1}}}}}

print">cueing bitmap reader"
bitmap_reader{{{cue={file=bitmap}}}}

print">cueing crawl reader"
crawl_reader{{{cue={file=crawl}, show_crawl={iterator={x=1}, loop=true}}}}

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

-- that's it!