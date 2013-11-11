-- this is the workflow example as found in the dsx hapi documentation folder

-- get the dsx module
local dsx = require'hapi.dsx'

-- get a critical section
local critical_section = dsx.new_cs()
local g_status_pos = -1
local playlist
local profile = "480i_30M_4x3"
local file_reader, live_window
local pump
local sync_status = {}
local clock
local file_reader_coroutine
local file_reader_initial_state
local result
local errors = {}

print=dsx.output_debug

-- create the board
print(">creating board #"..dsx.boards[1].serial_number)
dsx.boards[1](profile)

clock = dsx.boards[1].clock

-- create a pump
print">creating pump"
pump = dsx.pumps(clock)

playlist = {
	id = 1,							-- unique user-defined id number
	resolution = "480i_30M_4x3",	-- Matrox requires a base resolution for each playlist
	elements = {
		{
			name = "E:\\Beavis and Butthead.mpg",
			trim_in = 1,
			trim_out = 300
		},
	}
}

-- a complete file reader state
file_reader_initial_state = {
	id					= 1,	-- unique user-defined id number
	presentation_time	= clock.now,
	playlist			= playlist,
	position			= 1,
	position_increment	= 1,
	tdir				= false,
	mode				= "play",
	audio_scrub			= false,
}

file_reader_coroutine = coroutine.create(
	function(initial_presentation_time, filter_state, coroutine_arguments, initial_blocking, initial_errors) -- args coming from the hal
		-- the first resume will execute this line
		local presentation_time = initial_presentation_time
		local hal_arguments = {}
		local is_blocking = initial_blocking
		local errors = {} -- initial_errors
		local run = true

		-- I'm passing the initial state as coroutine arguments 
		filter_state = coroutine_arguments
		
		-- then we'll iterate below until we're ready to return
		repeat
			presentation_time, filter_state, hal_arguments, is_blocking, errors = coroutine.yield(filter_state)
			
			-- here we receive the current state and react based on the situation
			-- there's only one callback, this callback is used to report errors as well as to report normal operating status
			-- this is called once per frame from within the sequencer thread, it needs to return quickly else it will cause performance problems
			print(">status. presentation time: "..presentation_time..", sequence id: "..filter_state.playlist.id..", pos: "..filter_state.position..", increment: "..filter_state.position_increment..", tdir: "..tostring(filter_state.tdir)..", mode: "..filter_state.mode..", audio scrub: "..tostring(filter_state.audio_scrub))

			-- this sample code updates a global that holds the frame position... just as an example. Using a message passing mechanism would be more efficient.
			critical_section.lock=true
			g_status_pos = filter_state.position
			critical_section.lock=false
		-- assign run through hal_arguments...
		until run==false
		
		print">coroutine finished execution"
		-- returning false will tell the filter to shut down
		return result
	end )

-- create a file reader. (coroutine, arguments to first coroutine resume is the initial filter state)
print">creating file reader"
file_reader = dsx.boards[1].file_reader(pump, file_reader_coroutine, file_reader_initial_state)

local live_window_initial_state = {
	presentation_time = clock.now,
	create_window = true
}

local live_window_coroutine = coroutine.create( 
	function(initial_state) -- args coming from the hal
			-- the first resume will execute this line
			local presentation_time
			local filter_state = initial_state
			local hal_arguments
			local is_blocking
			local result = true

			-- then we'll iterate below until we're ready to return
			repeat
				result, presentation_time, filter_state, hal_arguments, is_blocking = coroutine.yield(filter_state, result)
				if (result) then
					-- did we get an error?
					-- cbcb TODO error handler
				end
			until result
			-- returning false will tell the filter to shut down
			return result
		end )

-- create a live window
print">creating live window"
live_window = dsx.boards[1].live_window(pump, live_window_coroutine, live_window_initial_state)

-- f(state, tv, blocking)
-- connect the live window to the file reader
print">connecting filters"
result, errors = live_window({sources={file_reader}}, 0, true)
if ((result==nil) and (#errors>0)) then
	print(">synchronous error report: "..errors[1]);
end

-- start the pump, this will start the playback. Note that we start the pump from the filter at the root of the hierarchy.
-- throttle at 1.0, software throttle based on the clock ticks
print">starting the pump"
pump({presentation_time=clock.now, pace=1, root=live_window}, 0, true)

dsx.sleep(10000)

-- play until the end of the file is reached
print">querying status and waiting"
critical_section.lock=true
while g_status_pos<300 do
	print(">main thread: status pos: "..g_status_pos)
	critical_section.lock=false

	-- don't query status too aggressively, good practice... I chose to wait one second between each call
	dsx.sleep(1000)
	
	-- DEADLOCK WARNING!!! careful when locking this thing
	critical_section.lock=true
end
critical_section.lock=false

-- we're done! stop the pump
print">stopping the pump"
pump({pace=false})

-- disconnect the filters
print">disconnecting filters"
live_window({source=nil}, 0, true)

-- that's it!