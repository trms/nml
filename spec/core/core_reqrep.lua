-- taken from nanomsg's reqrep.c
local busted = require("busted")
local nml = require'nml.core'

local socket_address = "inproc://test"
local sym = {AF_SP=-1, NN_REP=-1, NN_REQ=-1, EFSM=-1, NN_DONTWAIT=-1}
local i=0, found, res, rc, msg
local rep1, req1, req2

-- locate the constants from the symbol lookup
describe("symbol loading", function()
	found = true

	while found do
		found= nml.symbolinfo(i)
		if found and sym[found.name] then 
			sym[found.name] = found.value 
		end
		i=i+1
	end
	assert.is_true(sym.AF_SP ~= -1)
	assert.is_true(sym.NN_REP ~= -1)
	assert.is_true(sym.NN_REQ ~= -1)
	assert.is_true(sym.EFSM ~= -1)
end)

describe("REQREP #REQREP", function()
	-- Test req/rep with full socket types.
	it("creates a REP socket", function()
		rep1 = nml.socket(sym.AF_SP, sym.NN_REP)
		assert.is_truthy(rep1)
		assert.is_not_falsy(rep1)
	end)
	
	it("binds to the REP socket", function()
		assert.is_not_falsy(nml.bind(rep1, socket_address))
	end)

	it("creates a REQ socket", function()
		req1 = nml.socket(sym.AF_SP, sym.NN_REQ)
		assert.is_truthy(req1)
		assert.is_not_falsy( req1)
	end)

	it("connects the REQ socket", function()
		assert.is_not_falsy(nml.connect(req1, socket_address))
	end)

	it("creates a second REQ socket", function()
		req2 = nml.socket(sym.AF_SP, sym.NN_REQ)
		assert.is_truthy(req2)
		assert.is_not_falsy(req2)
	end)

	it("connects the REQ socket", function()
		assert.is_not_falsy(nml.connect(req2, socket_address))
	end)

	describe("test invalid sequence of sends and recvs #invalid", function()
		it("returns EFSM on invalid send", function()
			rc = nml.send(rep1, "ABC", 3, 0)
			assert.is_falsy( rc)
			assert.are_equal(sym.EFSM, nml.errno())
		end)

		it("returns EFSM on invalid recv", function()
			rc = nml.recv(req1, sym.NN_DONTWAIT)
			assert.is_falsy(rc)
			assert.are_equal(sym.EFSM, nml.errno())
		end)
	end)

	describe("test fair queueing requests", function()
		it("sends to req2", function()
			assert.is_not_falsy(nml.send(req2, "ABC", sym.NN_DONTWAIT))
		end)
		
		it("receives the message on rep1", function()
			msg = nml.recv(rep1, sym.NN_DONTWAIT)
			assert.is_not_falsy(msg)
			assert.are_equal(msg, "ABC") -- NOTE: this string is NOT null terminated, but it gets a temporary terminator on the C side
		end)
		
		it("sends to rep1", function()
			assert.is_not_falsy(nml.send(rep1, "ABC", sym.NN_DONTWAIT))
		end)

		it("receives the message on req2", function()
			nml.sleep(50) --wait so that msg is ready.
			msg, res = nml.recv(req2, sym.NN_DONTWAIT)
			assert.is_not_falsy(msg)
			assert.are_equal(msg, "ABC") -- NOTE: this string is NOT null terminated, but it gets a temporary terminator on the C side
		end)

		it("sends to req1", function()
			assert.is_not_falsy( nml.send(req1, "ABC", sym.NN_DONTWAIT))
		end)
		
		it("receives the message on rep1", function()
			nml.sleep(50) --wait so that msg is ready.
			msg = nml.recv(rep1, sym.NN_DONTWAIT)
			assert.is_not_falsy(msg)
			assert.are_equal(msg, "ABC") -- NOTE: this string is NOT null terminated, but it gets a temporary terminator on the C side
		end)

		it("sends to rep1", function()
			assert.is_not_falsy( nml.send(rep1, "ABC", sym.NN_DONTWAIT))
		end)

		it("receives the message on req1", function()

			nml.sleep(50) --wait so that msg is ready.
			msg = nml.recv(req1, sym.NN_DONTWAIT)
			assert.is_not_falsy(msg)
			assert.are_equal(msg, "ABC") -- NOTE: this string is NOT null terminated, but it gets a temporary terminator on the C side
		end)
	end)

	it("closes rep1", function()
		assert.is_true(true, nml.close(rep1))
	end)

	it("closes req1", function()
		assert.is_true(true, nml.close(req1))
	end)

	it("closes req2", function()
		assert.is_true(true, nml.close(req2))
	end)

end) -- describe("REQREP #REQREP", function()