-- taken from nanomsg's reqrep.c
local busted = require("busted")
local nml = require'nml'

local socket_address = "inproc://test"

-- Test req/rep with full socket types.
local rep1 = nml.socket("AF_SP", "NN_REP")
nml.bind(rep1, socket_address)

local req1 = nml.socket("AF_SP", "NN_REQ")
nml.connect(req1, socket_address)

local req2 = nml
nml.connect(req2, socket_address)

-- Check invalid sequence of sends and recvs.
local rc, err = nml.send(rep1, "ABC", 3, 0)
assert(rc == nil and err)
print("err1 is "..err)
--assert(nml.errno() == EFSM)
rc, err = nml.recv(req1, buf, sizeof (buf), 0)
assert(rc==-1 and err)
print("err2 is "..err)
--nn_assert (rc == -1 && nn_errno () == EFSM);

nml.send(req2, "ABC")
nml.recv(rep1, "ABC")
nml.send(rep1, "ABC")
nml.recv(req2, "ABC")

nml.send(req1, "ABC")
nml.recv(rep1, "ABC")
nml.send(rep1, "ABC")
nml.recv(req1, "ABC")

nml.close(rep1)
nml.close(req1)
nml.close(req2)