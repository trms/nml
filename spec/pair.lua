require'busted'
local nml = require'nml'
local pw = require'pl.pretty'.write

-- print('package', package.path)
-- print("sym is a "..pw(nml.sym))
local AF_SP = nml.sym.AF_SP.value
local NN_PAIR = nml.sym.NN_PAIR.value
local NN_DONTWAIT = nml.sym.NN_DONTWAIT.value
local SOCKET_ADDRESS = "inproc://a"
local sb
local sc

describe("test NN_PAIR #NN_PAIR", function()
	it("creates a first NN_PAIR socket", function()
		sb = nml.socket(AF_SP, NN_PAIR)
		assert.is_truthy(sb)
		assert.are_not_equal(-1, sb)	
	end)

	it("binds to the socket", function()
		assert.are_not_equal(-1, nml.bind(sb, SOCKET_ADDRESS))
	end)

	it("creates a second NN_PAIR socket", function()
		sc = nml.socket(AF_SP, NN_PAIR)
		assert.is_truthy(sc)
		assert.are_not_equal(-1, sc)
	end)

	it("connects to the socket address", function()
		assert.are_not_equal(-1, nml.connect(sc, SOCKET_ADDRESS))
	end)
	
	it("sends a message to sc", function()
		assert.are_not_equal(-1, nml.send(sc, "ABC", NN_DONTWAIT))
	end)

	it("sb receives the message", function()
		local result, message = nml.recv (sb, NN_DONTWAIT)
		assert.are_not_equal(-1, result)
		assert.are_equal("ABC", message)
	end)
	
	it("sends a message to sb", function()
		assert.are_not_equal(-1, nml.send(sb, "DEF", NN_DONTWAIT))
	end)

	it("sc receives the message", function()
		local result, message = nml.recv (sc, NN_DONTWAIT)
		assert.are_not_equal(-1, result)
		assert.are_equal("DEF", message)
	end)
	
	it("closes sc", function()
		assert.are_not_equal(-1, nml.close(sc))
	end)

	it("closes sb", function()
		assert.are_not_equal(-1, nml.close(sb))
	end)
end)