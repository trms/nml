local dsx = require'hapi.dsx'
local profile = "480i_30m_4x3"
local framerate = 29.97
local one_second_nanotime = 1 * 1000 * 1000 * 10

print = dsx.output_debug

print(">creating board")

-- this creates a board object (userdata) every time you call it...
local dsx_brd = dsx.boards[1]{profile=profile}

print(">fail construction test")

-- local dsx_brd_2 = dsx.boards[1]{profile=profile} -- this will fail because this index is already taken, can't create a second object there
-- it seems lua is smart and knows that there's already an object at boards[1], it doesn't call my __index method back

print(">second table access test..")

local dsx_brd_3 = dsx.boards[1] -- this will return the same user data as dsx_brd
print(">equal op: "..type(dsx_brd) .." / "..type(dsx_brd_3))
assert(dsx_brd==dsx_brd_3)

print(">assigning the clock")

-- I don't think we need to do this since the clock is bound to the board's resolution
local clock = assert(dsx_brd.clock)

--AAS:	Your saying that the board always has a clock and that we handed it the 
--		profile already, so no need to set it again. I agree. So, changing the profile
--		is done this way, but is not needed now, because it's already been set? Should
--		we change the board's timing profile in another way?
-- os.time // cbcb, load the table up
print(">res: "..clock.resolution)
print(">epoch host: "..clock.epoch.host.year.."/"..clock.epoch.host.month.."/"..clock.epoch.host.day.."   "..clock.epoch.host.hour..":"..clock.epoch.host.min.."."..clock.epoch.host.sec) -- when I Started up this was the time
print(">epoch dsx: "..clock.epoch.dsx) -- dsx time when we started
print(">now: "..clock.now.." nanotime") -- current dsx nanotime
print(">frequency: "..clock.cycle.." nanotime")

--AAS: Are all of these nanotime?

-- prints two return values:
-- 1: The clock value as a table with fractional seconds.
-- 2: The second return value is the DSX nano-time 
--		equivelent. Does not need to be zero. 

-- 	DST is always false.
--	Time is always GMT.

--[[ --> {
	year = 2011, 
	month = 5, 
	day = 21, 
	hour = 10,
	min = 3,
	sec = 51.237498273408,
	millis = 0

}, 0 
--]]

local ch = {filters={},}

-- The indexing is arbitrary and would be
-- implementation specific

local root_dir = "C:\\"

-- the playlist is not a dsx object, it's a table that gets populated on the lua side
-- cbcb will stack size become an issue if the playlist contains thousands of elements...?
-- AAS: no. 
local my_pl	= {
	resolution = profile,
	elements = {
		{
			name = root_dir.."12402Unity Care short filer 1.06.mov",
			trim_in = 1,
			trim_out = 300
			-- duration = 300-0, -- trim out minus trim in means playback at 1.0 rate	
		},
	}
}

print(">creating the pump")

-- create the dsx pump object shared across all filters inside a given tree
dsx.pump[1]{}

-- NOTE: I'll use the pump's resolution when creating the filters
print(">creating file_reader")
ch.filters.fr = dsx_brd.file_reader[1]{profile=profile, pump=dsx.pump[1]}
-- --> dsx_brd["file_reader"][1]{}
print(">creating live_window")
ch.filters.lw = dsx_brd.live_window[1]{profile=profile, create_internal_window=true, pump=dsx.pump[1]}

-- This is how I will call it using this metatable:
--[[
filter_index_mt.__call = function(filter_table, filter, ...) --var args are arguments to pass the hapi.filter
	local filter = filter{filter_table.channel.pump, filter, ...}

	return filter
end
--]]

--this is what the clock pumps.
--at start, the pump is not on.

--when issuing nano time and calculating lengths,
--the object will do the cycles to nano time transformation
--[[local okay, err = ch.filters.output{
	clock.now = {
		{no_input = "hold_last_frame"},
	},
}
--]]

--this should be the way that we deal with whether
--or not there is an issue with the commands as sent.
-- if not okay then return okay, err end

--i will write clock.to_dsx_time
--[[
local show_start = clock.to_dsx_time{
	year = 2011, 
	month = 5, 
	day = 21, 
	hour = 13 + CUR_TZ, --made up global I just shat in here.
	min = 0,
	sec = 0,
	ms = 0,
}
]]

print">calling add_source on lw"
ch.filters.lw{{{add_source = ch.filters.fr}}}

print">calling cue on fr"
ch.filters.fr{{{cue = {playlist = my_pl, start_pos = 1, iterator = {pos = 1}}}}}

-- test command piping
print">test command piping"
ch.filters.lw{
	[clock.now+one_second_nanotime] 
	= {	{add_source=ch.filters.fr}, 
		{remove_source=ch.filters.fr}, 
		{add_source=ch.filters.fr}, 
		{remove_source=ch.filters.fr}, 
		{add_source=ch.filters.fr}}}
clock{{{wait_until_nanotime=clock.now+5*one_second_nanotime}}}

-- start the channel, I don't think this needs to be timestampted
-- use this since we can't rely on a realtime output
print">starting the pump"
ch.filters.lw{[clock.now]={{pump=1.0}}}

-- get the status

-- one second from now
local status = {error=0, file=0, pos=0, rate=0}

function nanotime_to_frames(nanotime)
	return nanotime*framerate/one_second_nanotime
end

print(">"..os.date("%I:%M:%S%p").." - wait_until_nanotime. time now: "..nanotime_to_frames(clock.now))

-- wait until the specified time
local status_time = clock.now + 2*one_second_nanotime
print">calling wait #1"
clock{[clock.now]={{wait_until_nanotime=status_time}}}

--print(">"..os.date("%I:%M:%S%p").." -- wait_until_nanotime DONE. time now: "..nanotime_to_frames(clock.now))

while status.pos < 300 do

	-- get the state from the file reader
	status = ch.filters.fr{[clock.now]={{status={}}}}
	print(">status t:["..nanotime_to_frames(clock.now).."] pos:["..status.pos.."] rate:["..status.rate.."] file:["..status.file.."]")

	-- wait one second before calling again
	status_time = clock.now
	clock{[clock.now]={{wait_until_nanotime=status_time + one_second_nanotime}}}
end

-- AAS: One thing that I'm seeing is the creation of a lot of tables.
-- This is due to my suggestion of using a single table as an argument
-- list. Some ways to remedy this:
-- local args = {[now] = {{wait = {status_time}}}}
-- clock( args )
-- args[now] = {{do_stuff = {}}}

-- AAS: However, this only is one table of difference.
-- It would be nice if all command objects could be marked and
-- swept differently, such that the GC would be able to do it 
-- more efficiently.
-- Perhaps we should just wait and see if it becomes an issue.

-- stop the channel
print">stopping the pump"
ch.filters.lw{[clock.now]={{pump=false}}}

-- undo the tree
print">disconnecting filters"
ch.filters.lw{[clock.now] = {{remove_source="all"}}}

-- test garbage collection
collectgarbage("collect")