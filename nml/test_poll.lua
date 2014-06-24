local nml = require'nml'

local PAIR_ADDR = "inproc://pair"
local msg
local count



local function poll_check(sockets, time_out)
	local ready, sockets,  err = nml.poll(sockets, time_out)
	if not ready then
		print("poll errored:", err)
	else
		print(("poll: %s, recv: %d, send: %d"):format(ready, #sockets.recv, #sockets.send))
	end
	return ready, sockets, err
end

local pair_1 = assert(nml.pair())
local pair_2 = assert(nml.pair())

assert(pair_1:bind( PAIR_ADDR))
assert(pair_2:connect(PAIR_ADDR))

local sockets = {pair_1, pair_2}
local protect = sockets
local ready, sockets,  err = poll_check(sockets, 10)

print("Sending pair_1", pair_1:send("ABC"))
local ready, sockets,  err = poll_check(sockets, 10)
print("Sending pair_2", pair_2:send("DEF"))
-- print("TERMINATED")
-- nml.term()

local ready, sockets,  err = poll_check(sockets, 10)
if not ready then
	print("YAY!", err)
end
print("pair_2 recv", pair_2:recv())
local ready, sockets,  err = poll_check(sockets, 10)
print("pair_1 recv", pair_1:recv())
pair_1.events = "recv"
pair_2.events = "recv"
print("Events are now:", pair_1.events, pair_2.events)
local ready, sockets,  err = poll_check(sockets, 500)
local ready, sockets,  err = poll_check(sockets, 10)