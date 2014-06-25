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

local msgpack, ret_str, test_table, ret_table, ret_ud, err, msg_start_index

local msgpack_api = {
	"ser", --serialize
	"deser", --deserialize
}

describe ("msg_mspack operation #msg_mspack", function()
	it("can be loaded", function()
		msgpack = require'nml.msgpack.core'

	end)

	it("has all of the required methods", function()
		for i, method in ipairs(msgpack_api) do
			it("has all of the api calls present.", function()
				assert.is_truthy(msgpack[msgpack_api])
			end)
		end

	end)

	it("can make a new message with a table and return a string of the value.", function()
		test_table = {"Hello,", "world", "!"}
		ret_str = msgpack.ser(test_table)
--I'm not sure what the msgpack version of the above would be...
		assert.equal([=[["Hello,","world","!"]]=], ret_str)
		
	end)
	it("given a string, it can parse a msgpack encoded message and turn it into a lua table.", function()
		ret_table = msgpack.deser(ret_str)
		assert.equal(test_table, ret_table)
	end)

	it("given a table and an nml_msg, it can make a new message and return the same msg_ud object, but with the buffer filled in. The third argument is prepended to the message.", function()
		msg_ud = assert(nml.nml_msg())
		ret_ud, err = msgpack.ser(test_table, msg_ud, "msgpack\0")
		assert.equal(msg_ud, ret_ud) --they reference the same value.

		assert.equal("msgpack", nml.msg_gethead(msg_ud, "\0"))
		--length of type plus 1 (\0) plus 1 (first char of msgpack data)
		msg_start_index = #("msgpack") + 2
		assert.equal(ret_str, nml.msg_gettail(msg_ud, msg_start_index)) 

	end)



	it("given an nml_msg, it can parse the msgpack encoded data and turn it into a lua table.", function()
		ret_table = msgpack.deser(msg_ud, msg_start_index)
		assert.is_equal(test_table, ret_table)
	end)

	it("can encode a table that has a nested nml_msg userdata.", function()
		--todo
	end)



end)