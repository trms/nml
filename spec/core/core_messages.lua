local nml, events, AF_SP, NN_PAIR, PAIR_ADDR, msg, count, sockets, pair_1, pair_2, msg_ud, msg_ud_clone, msg_str1, msg_str2, msg_str3, message_api, test_suite

describe("messaging api #msg", function()
	setup(function()
		nml=require'nml'
		events = require'nml.events'

		AF_SP = nml.symbols.AF_SP.value
		NN_PAIR = nml.symbols.NN_PAIR.value

		nml = nml.core

		PAIR_ADDR = "inproc://pair"
		pair_1 = nml.socket(AF_SP, NN_PAIR)
		nml.bind(pair_1, PAIR_ADDR)
		pair_2 = nml.socket(AF_SP, NN_PAIR)
		nml.connect(pair_2, PAIR_ADDR)
		msg_str1 = "Hello, World!" 
		msg_str2 = "string\0My second string"
		msg_str3 = "string\0My third string"

		message_api = {
			"msg_getbuffer",	-- gets the buffer lud
			"msg_getheader",	-- gets the header as a lua string
			"msg_fromstring", 	--make message contents from a string
			"msg_frommessage", 	--make message contents from another message
			"msg_tostring", 	--turn any message into a Lua string.
			"msg_getsize", 		--number of bytes in a message. nil is no message present (not initialized).
			"msg_setheader",	-- copies the specified string in the message's header, used to id the protocol
			"msg_getheader",	-- returns the message's header string as a lua string
			"msg_free", 		--msg_free an existing message. 
								--true is "message freed", 
								--false is "no message present", 
								--nil, err is an error.
			"msg_alloc",		--allocates memory for a buffer. probably only called in C
			"msg_realloc", 		--dito, but reaollocates...
			"__tostring",
			"__len",
			"__gc",
		}
	end)

	before_each(function()
		-- -- -- -- -- 
		-- we'll reuse nml and msg_ud throughout the following tests
		msg_ud = nml.nml_msg()
		test_suite = {nml=nml, msg_ud=msg_ud}
	end)

	teardown(function()
		msg_ud = nil
		msg_ud_clone = nil
		collectgarbage();collectgarbage();collectgarbage()
	end)

	it("can make a new message that is empty.", function()
		--new empty message
		assert.is_truthy(msg_ud)

		-- is a userdata
		assert.is_equal("userdata", type(msg_ud))
		
		--this should call the __gc method, which should see that the buffer is nil and do nothing.
		--don't know how to test that though.
		msg_ud = nil
		collectgarbage()
	end)

	-- has not been set yet.
	it("reports an empty header", function()
		for _, v in pairs(test_suite) do
			assert.is_equal("", v.msg_getheader(msg_ud))
		end
	end)

	it("can have a string serialized into it.", function()
		for _, v in pairs(test_suite) do
			assert.is_truthy(v.msg_fromstring(msg_ud, msg_str1))
		end
	end)

	it("reports the correct message through tostring.", function()
		for _, v in pairs(test_suite) do
			v.msg_fromstring(msg_ud, msg_str1)
			assert.are_equal(msg_str1, tostring(msg_ud))
		end
	end)

	it("will let me re-make the buffer in an existing message.", function()
		for _, v in pairs(test_suite) do
			v.msg_fromstring(msg_ud, msg_str1)
			assert.is_truthy(v.msg_fromstring(msg_ud, msg_str2))
		end
	end)
	
	it("reports the correct message through tostring after re-making the buffer.", function()
		for _, v in pairs(test_suite) do
			v.msg_fromstring(msg_ud, msg_str1)
			v.msg_fromstring(msg_ud, msg_str2)
			assert.are_equal(msg_str2, tostring(msg_ud))
		end
	end) 

	it("reports the header as STR after using fromstring.", function()
		for _, v in pairs(test_suite) do
			v.msg_fromstring(msg_ud, msg_str1)
			assert.are_equal("STR ", v.msg_getheader(msg_ud)) -- 4 characters
		end
	end)

	it("will clone an existing message.", function()
		for _, v in pairs(test_suite) do
			msg_ud_clone = nml.nml_msg()

			v.msg_fromstring(msg_ud, msg_str3)
			v.msg_frommessage(msg_ud, msg_ud_clone)
		
			assert.are_equal(#msg_ud, #msg_ud_clone)
			assert.are_equal(tostring(msg_ud), tostring(msg_ud_clone))
		end
	end)
	
	it("will let me specify a header.", function()
		for _, v in pairs(test_suite) do
			assert.is_truthy(v.msg_setheader(msg_ud, "mypr"))
		end
	end)

	it("will let me retrieve the message's header into a lua string", function()
		for _, v in pairs(test_suite) do
			v.msg_setheader(msg_ud, "mypr")
			assert.are_equal("mypr", v.msg_getheader(msg_ud))
			v.msg_setheader(msg_ud, "")
		end
	end)

	it("returns the header as a type.", function()
		for _, v in pairs(test_suite) do
			v.msg_setheader(msg_ud, "mypr")
			assert.is_equal("mypr", v.msg_getheader(msg_ud))
		end
	end)

	it("can reproduce the entire message as a string", function()
		for _, v in pairs(test_suite) do
			v.msg_fromstring(msg_ud, msg_str3)
			assert.is_equal(msg_str3, v.msg_tostring(msg_ud))
		end
	end)

	it("can report the size of a message using the msg_getsize method", function()
		for _, v in pairs(test_suite) do
			v.msg_fromstring(msg_ud, msg_str3)
			assert.is_equal(#msg_str3, v.msg_getsize(msg_ud))
		end
	end)

	it("can report the size of a message using the __len metamethod", function()
		nml.msg_fromstring(msg_ud, msg_str3)
		assert.is_equal(#msg_str3, #msg_ud)
	end)
	
	it("can allow me to msg_free a message explicitly and not crash if it is collected (double msg_free).", function()
		-- use nml to free the ud and the message api to free the clone
		for _, v in pairs(test_suite) do
			v.msg_fromstring(msg_ud, msg_str1)

			--returns true when msg_free happens. false if no pointer. nil, msg (or error) if something bad happens.
			assert.is_true(v.msg_free(msg_ud))
		
			--msg_freeing again returns false.
			assert.is_false(v.msg_free(msg_ud))
			msg_ud = nml.nml_msg()
		end
	end)
end)