-- get the dsx module
local dsx = require'hapi.dsx'

print=dsx.output_debug

-- create the board
print">creating board"
dsx.boards[1]{profile="480i_30M_4x3"}

-- create a pump
print">creating pump"
dsx.pump[1]{}

print">creating output"
local out = dsx.boards[1].output{
	profile="480i_30M_4x3", 
	pump=dsx.pump[1], 
	buffering={frames=8}, 
	video={"sdi_1"}, -- supported as a string (ex:video="sdi_1"), or as a string table
	audio={"aes_1", "aes_2"}}

-- confirm the output's configuration infos
print(">"..out.video[1])
assert(out.video[1]=="sdi_1")
assert(#out.audio==2)
print(">"..out.audio[1])
print(">"..out.audio[2])
assert((out.audio[1]=="aes_1") or (out.audio[1]=="aes_2"))
assert((out.audio[2]=="aes_1") or (out.audio[2]=="aes_2"))

-- turn on component on output A
print">turning component on"
out{{{component=true}}}
assert(out.component==true)

print(">output component is now "..tostring(out.component))

print">turning component off"
out{{{component=false}}}
assert(out.component==false)

print(">output component is now "..tostring(out.component))

-- that's it!