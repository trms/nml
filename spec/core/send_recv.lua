--local llthreads = require'llthreads'
local nml=require'nml'
local events = require'nml.events'
--local pw=require'pl.pretty'.write
require'busted'

local AF_SP = nml.symbols.AF_SP.value
local NN_PAIR = nml.symbols.NN_PAIR.value

nml = nml.core

local PAIR_ADDR = "inproc://pair"
local msg
local count
local sockets 
-- local pair_1 = nml.pair():bind(PAIR_ADDR)
-- local pair_2 = nml.pair():connect(PAIR_ADDR)
local pair_1 = nml.socket(AF_SP, NN_PAIR)
nml.bind(pair_1, PAIR_ADDR)
local pair_2 = nml.socket(AF_SP, NN_PAIR)
nml.connect(pair_2, PAIR_ADDR)
local msg1, msg2

describe("Send tests #send #recv", function()

	it("can send and recv a simple text message, without defining the buffer length.", function()
		msg1 = "ABC"
		nml.send(pair_1, msg1)
		-- assert.not_falsy()
		msg2 = assert.not_falsy(nml.send(pair_2))
		print(msg1, msg2)
		
	end)
	
	
-- /*  Clean up. */
	it("closes the sockets", function()
		assert.is_truthy(nml.close(pair_2))
		assert.is_truthy(nml.close(pair_1))
	end)
end)