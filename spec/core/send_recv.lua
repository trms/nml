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
local msg1_ud, msg1_str ,msg_type1, msg2_ud, msg2_str, msg_type2


-- describe("Send tests #send #recv", function()


-- 	it("can send and recv a simple text message, without defining the buffer length.", function()
-- 		msg1 = "ABC"
		
-- 		assert.is_truthy(nml.send(pair_1, msg1))
		
-- 		msg2, msg_type2 = nml.recv(pair_2)

-- 		it("receives a message as userdata", function()
-- 			assert.not_falsy(msg2)
-- 			assert.is_equal("userdata", type(msg2))
-- 		end)

-- 		it("receives a message type as the 2nd return value", function()
-- 			assert.is_equal("string", msg_type2)
-- 		end)
-- 	end)
	
-- 	it("converts a message userdata using the nml.tostring() method.", function()
-- 		msg2_str = nml.tostring(msg2)
-- 		assert.isequal("string", type(msg2_str))
-- 	end)
-- 	it("frees the message, which is still there, even after it was converted.", function() 
		-- --returns true when free happens. false if no pointer. nil, msg (or error) if something bad happens.
		-- assert.is_true(nml.free(msg2))
		-- --freeing again returns false.
		-- assert.is_false(nml.free(msg2))
		-- msg2 = nil
		-- collectgarbage();collectgarbage();collectgarbage()
		-- --there shouldn't be any error here, even though __gc was called.
-- 	end)

-- 	it("can send and receive a string message where the length and message type are defined", function()
-- 		msg_type1 = "text/foo"
-- 		assert.is_truthy(nml.send(pair_1, msg1, 0, #msg1, msg_type1))
-- 		msg2, msg_type2 = nml.recv(pair_2)

-- 	end)
		

	

-- 	it("can send and recv a simple text message, without defining the buffer length.", function()
-- 		msg1 = "ABC"
-- 		nml.send(pair_1, msg1)
-- 		msg2 = nml.recv(pair_2)

-- 		assert.not_falsy(msg2)
-- 		assert.is_equal(msg1, msg2)

-- 	end)
	
	
-- -- /*  Clean up. */
-- 	it("closes the sockets", function()
-- 		assert.is_truthy(nml.close(pair_2))
-- 		assert.is_truthy(nml.close(pair_1))
-- 	end)
-- end)