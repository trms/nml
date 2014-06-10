-- taken from nanomsg's reqrep.c
--local busted = require("busted")
local nml = require'nml'

local socket_address = "inproc://test"
local sym = {AF_SP=-1, NN_REP=-1, NN_REQ=-1, EFSM=-1, NN_DONTWAIT=-1}
local i=0, found, res

-- locate the constants from the symbol lookup
repeat 
	found=select(2, nml.symbolinfo(i))
	if found and sym[found.name] then sym[found.name] = found.value end
	i=i+1
until ((sym.AF_SP~=-1 and sym.NN_REP~=-1 and sym.NN_REQ~=-1 and sym.EFSM~=-1) or not found)
assert(sym.AF_SP~=-1 and sym.NN_REP~=-1 and sym.NN_REQ~=-1 and sym.EFSM~=-1)

-- Test req/rep with full socket types.
local rep1 = nml.socket(sym.AF_SP, sym.NN_REP)
nml.bind(rep1, socket_address)

local req1 = nml.socket(sym.AF_SP, sym.NN_REQ)
nml.connect(req1, socket_address)

local req2 = nml.socket(sym.AF_SP, sym.NN_REQ)
nml.connect(req2, socket_address)

-- Check invalid sequence of sends and recvs.
local rc = nml.send(rep1, "ABC", 3, 0)
assert(rc==-1 and nml.errno()==sym.EFSM)

rc = nml.recv(req1, sym.NN_DONTWAIT)
assert(rc==-1 and nml.errno()==sym.EFSM)

nml.send(req2, "ABC", sym.NN_DONTWAIT)
nml.recv(rep1, sym.NN_DONTWAIT)
nml.send(rep1, "ABC", sym.NN_DONTWAIT)
nml.recv(req2, sym.NN_DONTWAIT)

nml.send(req1, "ABC", sym.NN_DONTWAIT)
nml.recv(rep1, sym.NN_DONTWAIT)
nml.send(rep1, "ABC", sym.NN_DONTWAIT)
nml.recv(req1, sym.NN_DONTWAIT)

nml.close(rep1)
nml.close(req1)
nml.close(req2)