local nml, events, AF_SP, NN_PAIR, PAIR_ADDR, msg, count, sockets, pair_1, eid1, pair_2, eid2, msg1_str, msg1_type, msg2_ud

describe("Send tests #sendrecv", function()
	setup(function()
		nml=require'nml'
		events = require'nml.events'

		AF_SP = nml.symbols.AF_SP.value
		NN_PAIR = nml.symbols.NN_PAIR.value

		nml = nml.core

		PAIR_ADDR = "inproc://pair"
		pair_1 = nml.socket(AF_SP, NN_PAIR)
		pair_2 = nml.socket(AF_SP, NN_PAIR)
		
		msg1_str = "string\0My first and last string"
		msg1_type = "TST1"
	end)

	before_each(function()
		msg = {}
		msg.buffer = nml.msg_fromstring(msg1_str)
		nml.msg_setheader(msg.buffer, msg1_type)

		eid1 = nml.bind(pair_1, PAIR_ADDR)
		eid2 = nml.connect(pair_2, PAIR_ADDR)
	end)

	after_each(function()
		-- flush the socket
		nml.shutdown(pair_1, eid1)
		nml.shutdown(pair_2, eid2)
		
		msg = nil -- this will __gc the buffer which calls msg_free
		msg2_ud = nil
		
		collectgarbage()
	end)

	teardown(function()
		nml.close(pair_1)
		nml.close(pair_2)
		collectgarbage();
	end)
	it("can send a simple text message.", function()
		assert.is_truthy(nml.send(pair_1, msg.buffer))
	end)
	
	it("can recv a simple text message.", function()
		local err
		nml.send(pair_1, msg.buffer)
		msg2_ud, err = nml.recv(pair_2)
		print(err)
		assert.is_truthy(msg2_ud)
	end)
	
	it("receives a message as userdata.", function()
		nml.send(pair_1, msg.buffer)
		msg2_ud = nml.recv(pair_2)
		assert.is_equal("userdata", type(msg2_ud))
	end)

	it("received a non-empty message.", function()
		nml.send(pair_1, msg.buffer)
		msg2_ud = nml.recv(pair_2)
		assert.is_true(#msg2_ud>0)
	end)

	it("received a message of the same type as the original.", function()
		nml.send(pair_1, msg.buffer)
		msg2_ud = nml.recv(pair_2)
		assert.is_equal(msg1_type, nml.msg_getheader(msg2_ud))
	end)

	it("received a message of the same size as the original.", function()
		local len=#msg.buffer
		nml.send(pair_1, msg.buffer)
		msg2_ud = nml.recv(pair_2)
		assert.are_equal(len, #msg2_ud)
	end)

	it("received the original message.", function()
		nml.send(pair_1, msg.buffer)
		msg2_ud = nml.recv(pair_2)
		assert.are_equal(msg1_str, nml.msg_tostring(msg2_ud))
	end)
	
	it("frees the message, which is still there, even after it was converted.", function() 
		nml.send(pair_1, msg.buffer)
		msg2_ud = nml.recv(pair_2)
		--returns true when free happens. false if no pointer. nil, msg (or error) if something bad happens.
		assert.is_true(nml.msg_free(msg2_ud))
		--freeing again returns false.
		assert.is_false(nml.msg_free(msg2_ud))
		msg2_ud = nil
		collectgarbage();collectgarbage();collectgarbage()
		--there shouldn't be any error here, even though __gc was called.
	end)
end)