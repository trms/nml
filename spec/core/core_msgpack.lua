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
	"pack", --serialize
	"unpack", --deserialize
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
		ret_str = msgpack.pack(test_table)
		assert.are_equal("string", type(ret_str))
	end)

	it("given a string, it can parse a msgpack encoded message and turn it into a lua table.", function()
		ret_table = msgpack.unpack(ret_str)
		assert.equal(test_table, ret_table)
	end)

	it("given a table and an nml_msg, it can populate the message and return the same msg_ud object, but with the buffer filled in.", function()
		msg_ud = nml.nml_msg()
		assert.is_truthy(msg_ud)

		ret_ud, err = msgpack.pack(test_table, msg_ud)
		assert.are_equal("userdata", type(ret_ud)) -- we got a msg_ud
		assert.are_equal(msg_ud, ret_ud) --they reference the same value.
	end)

	it("given an nml_msg, it can parse the msgpack encoded data and turn it into a lua table.", function()
		ret_table = msgpack.unpack(msg_ud)
		assert.is_equal(test_table, ret_table)
	end)

	it("can encode a table that has a nested nml_msg userdata.", function()
		--todo
	end)
end)