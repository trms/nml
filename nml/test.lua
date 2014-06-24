local nml = require'nml'

local PAIR_ADDR = "inproc://pair"
local msg
local count


local pair_1 = assert(nml.pair())
local pair_2 = assert(nml.pair())

assert(pair_1:bind( PAIR_ADDR))
assert(pair_2:connect(PAIR_ADDR))

local sockets = {pair_1, pair_2}
local protect = sockets

print("Sending pair_1:", pair_1:send("ABC"))

print("Recv pair_2:", pair_2:recv())

print("Sending pair_1:",pair_1.core.sendmsg(pair_1[1], "ABC"))

print("Recv pair_2:", pair_2.core.recvmsg(pair_2[1]))

