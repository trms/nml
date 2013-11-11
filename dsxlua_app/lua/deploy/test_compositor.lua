--[[

root <-- output <-- dma_out_av <-- compositor <-- file_reader_A
	  |				  |						   |	
	  -- live_window <-						   -- file_reader_B
]]

--require'lua_dev_path'
--_ENV = require'mediacircus'

-- get the dsx module
local usb_dongle = true

local dsx = require'hapi.dsx'
local pump
local crossfade_duration_seconds = 5
local crossfade_duration_frames = 30*crossfade_duration_seconds
local kf_current = {source=normalizer_B, zindex=199, top=0, left=0, opacity=0, audio={0,0}, vbi=false} -- initial foreground keyframe
local kf_target = {opacity=1.0}
local kf_increment = 1/crossfade_duration_frames
local kf_cs = dsx.new_cs()
local loop = true
local root, output, live_window, dma_out_av, compositor, file_reader_A, file_reader_B, normalizer_A, normalizer_B-- filters
local clock
local profile="480i_30M_4x3"
local width=720		-- frame size, used in the compositor keyframes
local height=486
local band=10
local trim_out = 300
local one_second_nanos = 1*1000*1000*10
local filename_A = "E:\\Beavis and Butthead.mpg"
local filename_B = "E:\\12402Unity Care short filer 1.06.mov"

local playlist_A = {
	resolution = profile,
	elements = {
		{ name = filename_A, trim_in = 1, trim_out = trim_out },
		{ name = filename_A, trim_in = 1, trim_out = trim_out },
		{ name = filename_A, trim_in = 1, trim_out = trim_out },
		{ name = filename_A, trim_in = 1, trim_out = trim_out },
		{ name = filename_A, trim_in = 1, trim_out = trim_out },
		{ name = filename_A, trim_in = 1, trim_out = trim_out }
	}
}
local playlist_B = {
	resolution = profile,
	elements = {
		{ name = filename_B, trim_in = 1, trim_out = trim_out },
		{ name = filename_B, trim_in = 1, trim_out = trim_out },
		{ name = filename_B, trim_in = 1, trim_out = trim_out },
		{ name = filename_B, trim_in = 1, trim_out = trim_out },
		{ name = filename_B, trim_in = 1, trim_out = trim_out },
		{ name = filename_B, trim_in = 1, trim_out = trim_out }
	}
}

print=dsx.output_debug

-- this will print the current layer state as a formatted command
-- fyi command format is: {[clock_tv] = {{layer={source=file_reader_A, zindex=100, top=0, left=0, opacity=1.0}},
local fn_status = function (state)
	--pt(state)
	local tv, commands = next(state, nil)
	print(">tv: "..tv.." opacity: "..commands[1]["layer"]["opacity"].."/"..commands[2]["layer"]["opacity"])
	return state
end

-- NOTE: this is the layer command callback
-- NOTE: this callback is called from within the compositor just as it's about to program it's compositing. It needs to return QUICKLY else realtimeness will be affected.
-- NOTE: it's possible to set the source to nil in the iterator, meaning to disconnect them. But it's IMPOSSIBLE to change the sources in any other ways inside the iterator.
-- NOTE: the iterator callback has to be inside a table. The table's lifespan controls the callback's lua state lifespan. 
--		 (a new L thread is attached to the callback table's mt).
local fn_crossfade = { function (state)

	local ret = state
	
	-- we receive the full compositor layout in parameter
	-- {nano_time = {{layer_table}, {layer_table}...}}
	-- nano_time: layer keyframe presentation time
	-- layer_table: the compositor layers
	
	local tv, commands = next(state, nil)

	print(">commands size is "..#commands)
	
	if #commands == 1 then
		-- we're done! stop calling 
		-- note: I can't return nil and remove the background in the same callback session, do it in 2 steps
		ret = nil
	else
		local background = commands[1]["layer"]
		local foreground = commands[2]["layer"]
		
		background["audio"][2] = background["audio"][2] - kf_increment
		background["audio"][2] = math.max(background["audio"][2], 0) -- clip to 0

		foreground["opacity"] = foreground["opacity"] + kf_increment -- math.min(foreground["opacity"] + kf_increment, 1)		
		foreground["opacity"] = math.min(foreground["opacity"], 1) -- clip to 1
		
		foreground["audio"][2] = foreground["audio"][2] + kf_increment
		foreground["audio"][2] = math.min(foreground["audio"][2], 1)
		
		kf_cs.lock = true
		kf_current = foreground
		kf_cs.lock = false
				
		-- if we're done with the crossfade, disconnect the first layer! this is awesome!
		if foreground["opacity"] >= kf_target["opacity"] then
			-- remove the background
			commands[1]["layer"] = nil
		end

		print(">crossfade progression="..foreground["opacity"].."/"..kf_target["opacity"])
	end

	-- other fields remain as-is
	-- return the new keyframe
	return ret
end }

-- create the board
print(">creating board #"..dsx.boards[1].serial_number)
dsx.boards[1]{profile=profile}
clock=dsx.boards[1].clock

-- create a pump
print">creating pump"
pump = dsx.pump()

-- root
print">creating root"
root=dsx.boards[1].root{pump=pump}

-- create a file reader
print">creating file readers"
file_reader_A=dsx.boards[1].file_reader{pump=pump}
file_reader_B=dsx.boards[1].file_reader{pump=pump}

-- compositor
print">creating compositor"
compositor=dsx.boards[1].compositor{pump=pump}

-- live window
print">creating live window"
live_window=dsx.boards[1].live_window{pump=pump, create_internal_window=true}

-- create normalizers
print">creating normalizers"
normalizer_A=dsx.boards[1].normalizer{pump=pump}
normalizer_B=dsx.boards[1].normalizer{pump=pump}

if (usb_dongle==false) then
	-- create the hardware filters
	print">creating dma out av"
	dma_out_av=dsx.boards[1].dma_out_av{pump=pump}

	print">creating output"
	output=dsx.boards[1].output{profile=profile, video={"sdi_1"}, buffering={frames=8}, pump=pump}

	print">connecting root to output"
	root{{{add_source=output}}}

	print">connecting output to dma"
	output{{{add_source=dma_out_av}}}

	print">connecting dma to compositor"
	dma_out_av{{{add_source=compositor}}}
end

-- connect the filters
-- NOTE: this is using a sw scaler window
--[[

root <-- output <-- dma_out_av <--- compositor <--- normalizer_A <-- file_reader_A
	  |				  			 |			     |	
	  ------------ live_window <--				 -- normalizer_B <-- file_reader_B
]]

print">connecting root to live window"
root{{{add_source=live_window}}}

print">connecting live window to compositor"
live_window{{{add_source=compositor}}}

-- clock.now is used here to help lisibility (all those brackets make the code hard to read)
-- tell the compositor which layer the sources belong to
-- 1-based except opacity where 0 is transparent
-- the compositor uses a sequence to index the sources (consecutive numeric index keys)
-- note: it's possible to have holes in the sequence as long as the holes are qualified. ex: [1] = nil, [2] = {}
print">connecting compositor to file readers"
compositor{[clock.now] ={	{add_source=normalizer_A},
							{add_source=normalizer_B}, 
							{layer={source=normalizer_A, zindex=100, top=0, left=0, opacity=1, audio={1,1}, vbi=true}}, -- audio volume initial and final (first and last sample volume value)
							{layer={source=normalizer_B, zindex=199, top=0, left=0, opacity=0, audio={0,0}, vbi=false}},
							{iterator = fn_status} }} -- one callback handles all layers, so it sits outside of the layer command

print">connecting normalizers"
normalizer_A{{{add_source=file_reader_A}}}
normalizer_B{{{add_source=file_reader_B}}}

print">cueing reader A"
file_reader_A{{{cue={playlist=playlist_A, start_pos=1, iterator={pos=1}}}}}

print">cueing reader B"
file_reader_B{{{cue={playlist=playlist_B, start_pos=1, iterator={pos=1}}}}}

-- start the pump, this will start the playback. Note that we start the pump from the filter at the root of the hierarchy.
-- throttle at 1.0, software throttle based on the clock ticks
print">starting the pump"
local pumpStart = clock.now+2*one_second_nanos
root{{{pump={presentation_time=pumpStart, pace=1}}}}

-- this will crossfade in and out layer 2 only, layer 1 keeps playing in the background
print(">wait 5 seconds.")
clock{{{wait_until_nanotime=clock.now+5*one_second_nanos}}}

-- crossfade
print">sending a new crossfade iterator"
compositor{{{iterator = fn_crossfade}}}

-- check against the status
-- timeout if an error occured, give it double the crossfade duration to complete the crossfade
repeat
	clock{{{wait_until_nanotime=clock.now+one_second_nanos}}}

	-- loop until the target opacity has been reached
	kf_cs.lock = true
	print(">main loop. opacity: "..kf_current["opacity"].."/"..kf_target["opacity"])
	loop = kf_current["opacity"] >= kf_target["opacity"]
	kf_cs.lock = false
until loop

-- wait a bit, tests the layer removal code
clock{{{wait_until_nanotime=clock.now+5*one_second_nanos}}}

-- we're done! stop the pump
print">stopping the pump"
root{{{pump=0}}}

print">disconnecting filters"

-- disconnect the filters
root{{{remove_source="all"}}}

if usb_dongle == false then
	output{{{remove_source="all"}}}
	dma_out_av{{{remove_source="all"}}}
end
live_window{{{remove_source="all"}}}
compositor{{{remove_source="all"}}}
normalizer_A{{{remove_source="all"}}}
normalizer_B{{{remove_source="all"}}}

-- that's it!