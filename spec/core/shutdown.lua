-- taken from nanomsg's reqrep.c
local busted = require("busted")
local nml = require'nml'.core

local sym = {AF_SP=-1, NN_REQ=-1}
local s
local eid
local rc

-- locate the constants from the symbol lookup
describe("symbol loading", function()
	local found=true
	local i=0

	while found do
		found= nml.symbolinfo(i)
		if found and sym[found.name] then 
			sym[found.name] = found.value 
		end
		i=i+1
	end
	assert.is_true(sym.AF_SP ~= -1)
	assert.is_true(sym.NN_REQ ~= -1)
end)

describe("shutdown tests", function()
	it("creates a REQ socket", function()
		s = nml.socket (sym.AF_SP, sym.NN_REQ)
		assert.is_truthy(s)
	end)

	it("connects to the socket", function()
		eid = nml.connect(s, "tcp://127.0.0.1:5590")
		assert.is_truthy(eid)
	end)

	it("removes the end point from the socket", function()
		rc = nml.shutdown (s, eid)
		assert.is_true(rc)
	end)

	it("closes the socket", function()
		rc = nml.close(s)
		assert.is_true(rc)
	end)
end)