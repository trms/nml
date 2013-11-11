-- create multiple filters of the same type and make sure they can be controlled independently
-- (lua metatables should be unique per filter)

-- get the dsx module
local dsx = require'hapi.dsx'

-- get a critical section
local targets = 3						-- number of playbacks

local file_reader = {}
local cs = dsx.new_cs()
local g_status_pos = 0
local my_pl = {}
local files = {
	"c:\\12402Unity Care short filer 1.06.mov",
	"c:\\Beavis and Butthead.mpg",
	"c:\\PROMO-FAQ ALG 06.mpg",}

print=dsx.output_debug

print">creating board"

-- create the board
dsx.boards[1]{profile="480i_30M_4x3"}	

for i=1, targets, 1 do

	print("loop #"..i)

	my_pl[i] = {
		resolution = "480i_30M_4x3",
		elements = {
			{
				name = files[i],
				trim_in = 1,
				trim_out = 300
			},
		}
	}

	print(">creating pump #"..i)

	-- create a pump
	dsx.pump[i]{}

	
	-- create a file reader
	if i==targets then
		print(">creating file reader (with status cb) #"..i)
		file_reader[i] = dsx.boards[1].file_reader{profile="480i_30M_4x3", pump=dsx.pump[i], callbacks={status=
			function (in_status)
				print("callback pos: "..in_status.pos)
					cs.lock=true
					g_status_pos=in_status.pos
					cs.lock=false
			end}}
	else
		print(">creating file reader #"..i)
		file_reader[i] = file_reader{profile="480i_30M_4x3", pump=dsx.pump[i]}
	end
	
	print(">creating live window #"..i)

	-- create a live window
	local live_window = dsx.boards[1].live_window{profile="480i_30M_4x3", create_internal_window=true, pump=dsx.pump[i]}

	print(">connecting filters #"..i)

	-- connect the live window to the file reader
	live_window{{{add_source=file_reader[i]}}}

	print(">cueing reader #"..i)

	-- cue the file reader. Note: replace file with a valid filename
	file_reader[i]{{{cue={playlist=my_pl[i], start_pos=1, iterator={pos=1}}}}}

	print(">starting the pump #"..i)

	-- start the pump, this will start the playback. Note that we start the pump from the filter at the root of the hierarchy.
	-- throttle at 1.0, software throttle based on the clock ticks
	live_window{{{pump={presentation_time=dsx.boards[1].clock.now, pace=1}}}}

	print(">querying status and waiting #"..i)
end

-- play until the end of the file is reached
cs.lock=true
while g_status_pos<300 do
	print(">status pos: "..g_status_pos)
	cs.lock=false

	-- don't query status too aggressively, good practice... I chose to wait one second between each call
	dsx.boards[1].clock{[dsx.boards[1].clock.now]={{wait_until_nanotime=dsx.boards[1].clock.now+1*1000*1000*10}}}
	cs.lock=true
end
cs.lock=false

for i=1, targets, 1 do
	
	print(">stopping the pump #"..i)

	-- we're done! stop the pump
	live_window{{{pump=0}}}

	print(">disconnecting filters #"..i)

	-- disconnect the filters
	live_window{{{remove_source="all"}}}
end

-- that's it!