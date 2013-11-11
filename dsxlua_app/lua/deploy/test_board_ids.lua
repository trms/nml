-- get the dsx module
local ANDY = false
local dsx = require'hapi.dsx'

print= not ANDY and dsx.output_debug or print

print"looking up the available dsx hardware resources..."

for index=1, #dsx.boards do
	print("board #"..index..", type: "..dsx.boards[index].series..", profile: "..dsx.boards[index].profile..", serial #"..dsx.boards[index].serial_number)
end

dsx.boards[1]{profile="480i_30M_4x3"}
assert(type(dsx.boards[1])=="table")
assert(dsx.boards[1]==dsx.boards[dsx.boards[1].serial_number]) -- can be indexed by serial number
print("board id "..dsx.boards[1].serial_number.." created, type is "..type(dsx.boards[1]))

-- ex:
-- board.outputs[1].video = {"sdi", "composite"}
-- board.outputs[1].audio = {"embedded_1", "aes_1", "analog_1"}
-- #board.outputs -- number of supported output IOs
-- audio resources can usually be allocated on different channels, but are mutually exclusive
-- ex: aes_1 will be enumerated on all outputs, but if aes_1 is assigned to channel A then it can't also be assigned to channel B

print("number of output IOs: "..#dsx.boards[1].outputs)
print("number of input IOs: "..#dsx.boards[1].inputs)

for i=1, #dsx.boards do

	-- enumerate outputs
	for j=1, #dsx.boards[i].outputs do
		print(">board #"..i.." output#"..j.."\n video outputs:")
		for k=1, #dsx.boards[i].outputs[j].video do
			print("  "..dsx.boards[i].outputs[j].video[k])
		end
	end
	
	print(">audio outputs:")
	for k=1, #dsx.boards[i].outputs.audio do
		print("  "..dsx.boards[i].outputs.audio[k])
	end

	-- enumerate inputs
	for j=1, #dsx.boards[i].inputs do
		print(">board #"..i.." input#"..j.."\n video inputs:")
		for k=1, #dsx.boards[i].inputs[j].video do
			print("  "..dsx.boards[i].inputs[j].video[k])
		end
	end
	print(">audio inputs:")
	for k=1, #dsx.boards[i].inputs.audio do
		print("  "..dsx.boards[i].inputs.audio[k])
	end
end

-- the end