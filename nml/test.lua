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
local ready, sockets,  err = nml.poll(sockets, 10)

print("Test:", ready, err)
print("revents:", sockets[1].revents,  sockets[2].revents)
