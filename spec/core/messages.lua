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
local msg_ud, msg_str

local message_api = {
 	"nml_msg", 		-- make a new message
 	"sethead", 		-- set the header byte string of message. Fixed size.
 	"gethead", 		-- get the header byte string of message. Fixed size.
 	"fromstring", 	--make message contents from a string
 	"tostring", 	--turn any message into a Lua string.
 	"getsize", 		--number of bytes in a message. nil is no message present (not initialized).
 	"free", 		--free an existing message. 
 						--true is "message freed", 
 						--false is "no message present", 
 						--nil, err is an error.
 	"alloc",		--allocates memore for a buffer. probably only called in C
 	"realloc", 		--dito, but reaollocates...
 	--not sure what other memory functions are needed...
}
describe("the nml_msg api methods #nml_msg", function()
	
		for i, method in ipairs(message_api) do
			it("has all of the api calls present.", function()
				assert.is_equal("function", type(nml[message_api]))
			end)
		end



end)
describe ("basic nml_msg operation #nml_msg", function()
	it("can make a new message that is empty.", function()
		--new empty message
		msg_ud = nml.nml_msg()

		assert.is_equal("userdata", type(msg_ud))
		--this is so that I can see what type it is, in unadorned Lua
		--also note that colon syntax works here too. I'll use that later.
		--TODO: implement this in nml, not in nml.core
		-- assert.is_equal("nml_msg", nml.type(msg_ud))
		-- has not been set yet.
		assert.is_equal(nil, nml.gethead(msg_ud))

		--this should call the __gc method, which should see that the buffer is nil and do nothing.
		--don't know how to test that though.
		msg_ud = nil
		collectgarbage()
	end)


	it("can make a new empty message and then have a string serialized into it.", function()
		--new empty message
		msg_ud = nml.nml_msg() 
		msg_str ="Hello, World!" 
		assert.is_truthy(nml.fromstring(msg_ud, msg_str)) --,#msg_str ) -- do we want to allow setting the length seperately?
	end)

	it("will let me re-make the buffer in an existing message.", function()
		msg_str = "string\0My second string"
		assert.is_truthy(nml.fromstring(msg_ud, msg_str))
	end) 

	it("will let me get the first seven characters of a message.", function()
		assert.is_equal("string\0", nml.gethead(msg_ud, 7))
	end)

	it("will let me get all of the characters up to and including a specified character.", function()
		assert.is_equal("string\0", nml.gethead(msg_ud, "\0"))
	end)
	it("will let me get all of the characters *after* a specified numerical index (1 based), *including* that index.", function()
		assert.is_equal("My second string", nml.gettail(msg_ud, 8))
	end)
	it("will let me get all of the characters *after* a specified character, *excluding* that character.", function()
		assert.is_equal("My second string", nml.gettail(msg_ud, "\0"))
	end)


	it("can reproduce the entier message as a string", function()
		assert.is_equal(msg_str, nml.tostring(msg_ud))
	end)

	it("can report the size of a message using the getsize method", function()
		assert.is_equal(#msg_str, nml.getsize(msg_ud))
	end)

	--I can do this, if needed.

	-- it("can report the size of a message using the __len metamethod", function()
	-- 	assert.is_equal(#msg_str, #msg_ud)
	-- end)
	

	it("can allow me to free a message explicitly and not crash if it is collected (double free).", function()
		--returns true when free happens. false if no pointer. nil, msg (or error) if something bad happens.
		assert.is_true(nml.free(msg_ud))
		--freeing again returns false.
		assert.is_false(nml.free(msg_ud))
		msg_ud = nil
		collectgarbage();collectgarbage();collectgarbage()
	--there shouldn't be any error here, even though __gc was called.

	end)

	--Christian: should we do this? That is, allow making a new message and defining the contents from a string?
	it("can make a new message from a string.", function()
		--new empty message
		msg_ud = nml.nml_msg("Hello, world.", "string") 
		--string is there
		assert.is_equal("string", nml.gethead(msg_ud))
	
		msg_ud = nil
		collectgarbage()
	end)


end)