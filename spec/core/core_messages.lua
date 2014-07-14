local nml, msg_str1, msg_str2, msg_str3, msg

--[[
	nml core message api:

			"msg_getbuffer",	-- gets the buffer ud
			"msg_getheader",	-- gets the header as a lua string
			"msg_fromstring", 	--make message contents from a string
			"msg_frommessage", 	--make message contents from another message's content
			"msg_tostring", 	--turn any message into a Lua string.
			"msg_getsize", 		--number of bytes in a message. nil is no message present (not initialized).
			"msg_setheader",	-- copies the specified string in the message's header, used to id the protocol
			"msg_free", 		--msg_free an existing message. 
								--true is "message freed", 
								--false is "no message present", 
								--nil, err is an error.
			"msg_alloc",		--allocates memory for a buffer. probably only called in C
			"msg_realloc", 		--dito, but reaollocates...
]]

describe("messaging api #msg", function()
	setup(function()
		nml=require'nml'.core

		msg_str1 = "Hello, World!" 
		msg_str2 = "string\0My second string"
		msg_str3 = "string\0My third string"
	end)

	before_each(function()
		msg = {}
	end)

	-- this will free the msg.buffer after every "it" test below
	after_each(function()
		-- make sure to free up the buffer properly
		if msg.buffer then
			nml.msg_free(msg.buffer)
		end
		msg = nil
		collectgarbage()
	end)

	teardown(function()
		collectgarbage();collectgarbage();collectgarbage()
	end)

	it("can make a new message that is empty.", function()
		--new empty message
		msg.buffer = nml.msg_alloc(0)
		assert.is_truthy(msg.buffer)

		-- is a userdata
		assert.is_equal("userdata", type(msg.buffer))
	end)

	-- has not been set yet.
	it("reports an empty header", function()
		msg.buffer = nml.msg_alloc(0)
		assert.is_equal("", nml.msg_getheader(msg.buffer))
	end)

	it("can have a string serialized into it.", function()
		msg.buffer = nml.msg_fromstring(msg_str1)
		assert.is_truthy(msg.buffer)
	end)
	
	it("reports the correct message through tostring.", function()
		msg.buffer = nml.msg_fromstring(msg_str1)
		-- this will call the buffer's __tostring metatable function
		assert.are_equal(msg_str1, tostring(msg.buffer))
	end)

	it("reports the header as STR after using fromstring.", function()
		msg.buffer = nml.msg_fromstring(msg_str1)
		assert.are_equal("STR ", nml.msg_getheader(msg.buffer)) -- 4 characters
	end)

	it("will clone an existing message.", function()
		msg.buffer = nml.msg_fromstring(msg_str3)

		local mt = getmetatable(msg.buffer)

		-- important!
		mt.getbuffer = nml.msg_getbuffer
		mt.getsize = nml.msg_getsize
		mt.getheader = nml.msg_getheader

		-- this will call getbuffer, getsize and getheader to get the job done
		local buffer2 = nml.msg_frommessage(msg.buffer)
		
		-- uses the __len operator
		assert.are_equal(#msg.buffer, #buffer2)
		-- uses the __tostring operator
		assert.are_equal(tostring(msg.buffer), tostring(buffer2))
	end)
	
	it("will let me specify a header.", function()
		msg.buffer = nml.msg_fromstring(msg_str1)
		assert.is_truthy(nml.msg_setheader(msg.buffer, "mypr"))
	end)

	it("will let me retrieve the message's header into a lua string", function()
		msg.buffer = nml.msg_fromstring(msg_str1)
		nml.msg_setheader(msg.buffer, "mypr")
		assert.are_equal("mypr", nml.msg_getheader(msg.buffer))
		nml.msg_setheader(msg.buffer, "")
	end)

	it("returns the header as a type.", function()
		msg.buffer = nml.msg_fromstring(msg_str1)
		nml.msg_setheader(msg.buffer, "mypr")
		assert.is_equal("mypr", nml.msg_getheader(msg.buffer))
	end)

	it("can reproduce the entire message as a string", function()
		msg.buffer = nml.msg_fromstring(msg_str3)
		assert.is_equal(msg_str3, nml.msg_tostring(msg.buffer))
	end)

	it("can report the size of a message using the msg_getsize method", function()
		msg.buffer = nml.msg_fromstring(msg_str3)
		assert.is_equal(#msg_str3, nml.msg_getsize(msg.buffer))
	end)

	it("can report the size of a message using the __len metamethod", function()
		msg.buffer = nml.msg_fromstring(msg_str3)
		assert.is_equal(#msg_str3, #msg.buffer)
	end)
	
	it("can allow me to msg_free a message explicitly and not crash if it is collected (double msg_free).", function()
		-- use nml to free the ud and the message api to free the clone
		msg.buffer = nml.msg_fromstring(msg_str1)

		--returns true when msg_free happens. false if no pointer. nil, msg (or error) if something bad happens.
		assert.is_true(nml.msg_free(msg.buffer))
		
		--msg_freeing again returns false.
		assert.is_false(nml.msg_free(msg.buffer))
	end)

	it("returns the data buffer", function()
		msg.buffer = nml.msg_fromstring(msg_str1)
		assert.are_equal("userdata", type(msg.buffer))
	end)
end)