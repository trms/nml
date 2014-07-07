local nml=require'nml'
local events = require'nml.events'
require'busted'

local AF_SP = nml.symbols.AF_SP.value
local NN_PAIR = nml.symbols.NN_PAIR.value

nml = nml.core

local PAIR_ADDR = "inproc://pair"
local msg
local count
local sockets 
local pair_1 = nml.socket(AF_SP, NN_PAIR)
nml.bind(pair_1, PAIR_ADDR)
local pair_2 = nml.socket(AF_SP, NN_PAIR)
nml.connect(pair_2, PAIR_ADDR)
local msg1_ud, msg1_str, msg1_type, msg2_ud

msg1_ud = nml.nml_msg()
msg1_str = "ABC"
msg1_type = "TST1"

nml.msg_fromstring(msg1_ud, msg1_str)
nml.msg_setheader(msg1_ud, msg1_type)

describe("Send tests #send #recv", function()
	it("can send a simple text message.", function()
		assert.is_truthy(nml.send(pair_1, msg1_ud))
	end)

	it("can recv a simple text message.", function()
		msg2_ud = nml.recv(pair_2)
		assert.is_truthy(msg2_ud)
	end)

	it("receives a message as userdata.", function()
		assert.is_equal("userdata", type(msg2_ud))
	end)

	it("received a non-empty message.", function()
		assert.is_true(#msg2_ud>0)
	end)

	it("received a message of the same type as the original.", function()
		assert.is_equal(msg1_type, nml.msg_getheader(msg2_ud))
	end)

	it("received a message of the same size as the original.", function()
		assert.are_equal(#msg1_ud, #msg2_ud)
	end)

	it("received the original message.", function()
		assert.are_equal(msg1_str, nml.msg_tostring(msg2_ud))
	end)
	
	it("frees the message, which is still there, even after it was converted.", function() 
		--returns true when free happens. false if no pointer. nil, msg (or error) if something bad happens.
		assert.is_true(nml.msg_free(msg2_ud))
		--freeing again returns false.
		assert.is_false(nml.msg_free(msg2_ud))
		msg2_ud = nil
		collectgarbage();collectgarbage();collectgarbage()
		--there shouldn't be any error here, even though __gc was called.
	end)

-- /*  Clean up. */
	it("closes the sockets", function()
		assert.is_truthy(nml.close(pair_2))
		assert.is_truthy(nml.close(pair_1))
	end)
end)