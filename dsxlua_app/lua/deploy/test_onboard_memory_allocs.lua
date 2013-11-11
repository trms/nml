--[[

SDI out A 
SDI out B 

]]

-- get the dsx module
local dsx = require'hapi.dsx'

print=dsx.output_debug

-- create the board
print">creating board"
dsx.boards[1]{profile="480i_30M_4x3"}

-- create a pump
print">creating pump"
dsx.pump[1]{}

print">creating output #1 - sdi_1+embedded+aes_1+aes_2"
local output_1 = output{profile="480i_30M_4x3", pump=dsx.pump[1], buffering={frames=8}, video="sdi_1", audio={"embedded_1", "aes_1", "aes_2"}} -- AES pairs 1 and 2 (4ch)

print">creating output #2 - sdi_2+embedded+aes_3"
local output_2 = output{profile="480i_30M_4x3", pump=dsx.pump[1], buffering={frames=10}, video="sdi_2", audio={"embedded_2", "aes_3"}} -- AES pair 3 (2ch)

print(">set board #1's buffering from "..output_1.buffering.frames.." to 0")
output_1{{{buffering={frames=0}}}}

-- do this a few times, this shouldn't be tried in real life scenarios since it may fragment the memory
for i=1, 10, 1 do
	print(">loop #"..i.. " board #1 buffering is "..output_1.buffering.frames.." frames")
	output_1{{{buffering={frames=i}}}}

	print(">loop #"..i.. " board #2 buffering is "..output_2.buffering.frames.." frames")
	output_2{{{buffering={frames=10-i}}}}
end

-- that's it!