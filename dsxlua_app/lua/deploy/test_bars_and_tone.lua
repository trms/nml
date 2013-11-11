-- get the dsx module
local dsx = require'hapi.dsx'
local live_window, bars-- filters
local pump

print=dsx.output_debug

-- create the board
print">creating board"
dsx.boards[1]{profile="480i_30M_4x3"}

-- create a pump
print">creating pump"
pump = dsx.pump[1]()

-- create the bars and tone filter
print">creating bars and tone"
bars=dsx.boards[1].bars_and_tone{pump=pump}

print">creating live window"
live_window=dsx.boards[1].live_window{create_internal_window=true, pump=pump}

print">connecting filters"

-- connect the filters
--[[
--		live_Window <--- bars
--]]

live_window{{{add_source=bars}}}

-- start the pump, this will start the playback. Note that we start the pump from the filter at the root of the hierarchy.
-- throttle at 1.0, software throttle based on the clock ticks
print">starting the pump"
live_window{{{pump={presentation_time=dsx.boards[1].clock.now, pace=1}}}}

-- wait a bit
dsx.boards[1].clock{{{wait_until_nanotime=dsx.boards[1].clock.now+10*1000*1000*10}}}

-- we're done! stop the pump
print">stopping the pump"
live_window{{{pump=0}}}

print">disconnecting filters"

-- disconnect the filters
live_window{{{remove_source="all"}}}

-- that's it!