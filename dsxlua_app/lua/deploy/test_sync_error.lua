-- test the ability to report synchronous errors on commands

-- get the dsx module
local dsx = require'hapi.dsx'
local message
local result
local brd
local pump
local fml
local profile = "480i_30m_4x3"
local file_reader
local file_writer
local playlist_A = {}
local file_av = "e:\\test_single_encode.mpg"
local time = os.date("*t", os.time())
local input
local output

print=dsx.output_debug

brd = dsx.board[1]

dsx.boards[1]{profile=profile}

pump = dsx.pump[1]{}

-- attempt to create an undefined filter
print">creating an undefined filter"
assert(dsx.boards[1].fml == nil)

-- create a filter without passing the pump
print">create a filter without passing the pump"
fml, message = dsx.boards[1].live_window{}
assert(fml == nil)
print(">message: "..message)

-- call the compositor commands with invalid or missing parameters	
do
	local compositor = dsx.boards[1].compositor{pump=pump}
	file_reader = dsx.boards[1].file_reader{pump=pump}

	result, message = compositor{{{add_source={source=file_reader, layer={top=0, left=0, opacity=1.0}}}}}
	assert(result == nil)
	print">missing layer parameter"
	print(">message: "..message)

	result, message = compositor{{{add_source={layer={zindex=1, top=0, left=0, opacity=1.0}}}}}
	assert(result == nil)
	print">missing source parameter"
	print(">message: "..message)

	result, message = compositor{{{add_source={source=file_reader, layer={zindex=0, top=0, left=0}}}}}
	assert(result == nil)
	print">missing opacity parameter"
	print(">message: "..message)

	result, message = compositor{{{add_source={source=file_reader, layer={zindex=1, left=0, opacity=1.0}}}}}
	assert(result == nil)
	print">missing top parameter"
	print(">message: "..message)
end

-- call dsx.fml
assert(dsx.fml == nil)

-- call a command like: lw{test}, lw{{test}}, lw{{{{test}}}}
do
	local clock = dsx.boards[1].clock

	-- this is a valid command
	clock{{{wait_until_nanotime=0}}}

	print">invalid command struture #1"
	result, message = clock{wait_until_nanotime=0}
	assert(result == nil)
	print(">message: "..message)

	print">invalid command struture #2"
	result, message = clock{{wait_until_nanotime=0}}
	assert(result == nil)
	print(">message: "..message)
end

-- recreate over an existing filter
do
	-- the error here is kinda weird but it makes sense
	local second_reader, result = dsx.boards[1].file_reader{pump=pump}
	print(">attempt to recreate on top of existing filter, result type is "..type(second_reader)..", message: "..message)

	-- shouldn't have affected the existing reader
	assert(second_reader == nil)
end

-- cue a reader with an empty playlist
print">cueing an empty playlist"
result, message = file_reader{{{cue={playlist=playlist_A, start_pos=1, iterator={pos=1}}}}}
assert(result == nil)
print(">message: "..message)

-- cue a reader with missing parameters...
playlist_A = {
	elements = {
		{
			name = "e:\\Beavis and Butthead.mpg",
			trim_in = 1,
			trim_out = test_duration_in_frames
		},
	}
}
print">cueing with missing parameters"
result, message = file_reader{{{cue={playlist=playlist_A, start_pos=1, iterator={pos=1}}}}}
assert(result == nil)
print(">message: "..message)

playlist_A = {
	resolution = profile,
}

result, message = file_reader{{{cue={playlist=playlist_A, start_pos=1, iterator={pos=1}}}}}
assert(result == nil)
print(">message: "..message)

result, message = file_reader{{{cue={start_pos=1, iterator={pos=1}}}}}
assert(result == nil)
print(">message: "..message)

-- test the file writer
file_writer = dsx.boards[1].file_writer{pump=pump}

print">cue without a filename"
result, message = file_writer{{{cue={
	filename_audio=file_av, 
	bitrate_mbps=5, 
	start_pos=0,
	buffering={frames=30}, -- file writer host pipeline size in video frames
	tc={hour=time.hour, minute=time.min, second=time.sec, frame=0},
	codec_video={encoder="mpeg2", profile="main", level="main", muxing="program"}, 
	codec_audio={encoder="mpeg1layer2"}}}}}
assert(result == nil)
print(">message: "..message)

print">cue without an encoder codec"
result, message = file_writer{{{cue={
	filename_video=file_av, 
	filename_audio=file_av, 
	bitrate_mbps=5, 
	start_pos=0,
	buffering={frames=30}, -- file writer host pipeline size in video frames
	tc={hour=time.hour, minute=time.min, second=time.sec, frame=0},
	codec_audio={encoder="mpeg1layer2"}}}}}
assert(result == nil)
print(">message: "..message)

-- test garbage collection if a filter is not fully constructed and its constructor returns nil..., should be destroyed properly
assert(dsx.boards[1].file_reader{} == nil)

-- create an input filter with missing parameters
print">creating an input with no profile parameter"
result, message = dsx.boards[1].input{
	pump=pump, 
	buffering={frames=8},
	video={"sdi_1"},
	audio={"embedded_1"}}
assert(result == nil)
print(">message: "..message)

print">creating an input with no buffering parameter"
result, message = dsx.boards[1].input{
	profile=profile, 
	pump=pump, 
	video={"sdi_1"},
	audio={"embedded_1"}}
assert(result == nil)
print(">message: "..message)

print">creating an input with no video parameter"
result, message = dsx.boards[1].input{
	profile=profile, 
	pump=pump, 
	buffering={frames=8},
	audio={"embedded_1"}}
assert(result == nil)
print(">message: "..message)

-- create an output filter with missing parameters
print">creating an output with no profile parameter"
result, message = dsx.boards[1].output{
	video={"sdi_1"}, 
	buffering={frames=8}, 
	pump=dsx.pump[1]}
assert(result == nil)
print(">message: "..message)

print">creating an output with no video parameter"
result, message = dsx.boards[1].output{
	profile=profile, 
	buffering={frames=8}, 
	pump=dsx.pump[1]}
assert(result == nil)
print(">message: "..message)

print">creating an output with no buffering parameter"
result, message = dsx.boards[1].output{
	profile=profile, 
	video={"sdi_1"}, 
	pump=dsx.pump[1]}
assert(result == nil)
print(">message: "..message)