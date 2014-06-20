--local llthreads = require'llthreads'
local nml=require'nml'
--local pw=require'pl.pretty'.write
require'busted'

local AF_SP = nml.symbols.AF_SP.value
local NN_PAIR = nml.symbols.NN_PAIR.value
local NN_DONTWAIT = nml.symbols.NN_DONTWAIT.value
local NN_POLLIN = 1 -- nml.sym.NN_POLLIN.value
local NN_POLLOUT = 2 -- nml.sym.NN_POLLOUT.value
local SOCKET_ADDRESS = "inproc://a"
local NN_RCVFD = nml.symbols.NN_RCVFD.value
local NN_SNDFD = nml.symbols.NN_SNDFD.value
local NN_SOL_SOCKET = nml.symbols.NN_SOL_SOCKET.value
local ETERM = nml.symbols.ETERM.value

nml = nml.core

local routine1 = [[
	local SOCKET_ADDRESS = ]]..SOCKET_ADDRESS..[[
	local nml=require'nml'
	local sc = nml.socket(AF_SP, NN_PAIR)
	nml.connect(sc, SOCKET_ADDRESS)
	nml.sleep (10)
	nml.send(sc, "ABC")
	nml.close(sc)]]

local routine2 = [[
	local nml=require'nml'
	nml.sleep (10)
	nml.term()]]

local NN_IN = 1
local NN_OUT = 2

local getevents = function(s, events, timeout)
    local rc;
    local pollset = {}
    local rcvfd;
    local sndfd;
	local tv = {}
	local revents;

    if events & NN_IN == NN_IN then
        rcvfd = nml.getsockopt(s, NN_SOL_SOCKET, NN_RCVFD)
		print("IN FD is:", rcvfd)
        assert(rcvfd)
		pollset = nml.FD_SET(rcvfd, pollset)
	end 

    if events & NN_OUT == NN_OUT then
        sndfd = nml.getsockopt(s, NN_SOL_SOCKET, NN_SNDFD)
        assert(sndfd)
		pollset = nml.FD_SET(sndfd, pollset)
    end

	--print(("getevents. events:[%d] rcvfd:[%d] sndfd:[%d] pollset:[%s]"):format(events, rcvfd and rcvfd or -1, sndfd and sndfd or -1, pw(pollset)))

    if timeout and timeout >= 0 then
        tv.tv_sec = timeout / 1000
        tv.tv_usec = (timeout % 1000) * 1000
	else
		tv = nil
    end
	-- only check for readability
	rc, pollset = nml.select(pollset, {}, {}, tv)
	assert(pollset)
    
	print(("-select done. pollset:[%s]"):format(pw(pollset)))

	revents = 0
	for i=1, #pollset do
		if pollset[i] == rcvfd and nml.FD_ISSET(rcvfd, pollset) then
			revents = NN_IN
		end
		if pollset[i] == sndfd and nml.FD_ISSET(sndfd, pollset) then
			revents = revents | NN_OUT
		end
	end
	--print("getevents returning "..revents)
    return revents
end

local rc
local sb
local sc
local buf = {}
local msg

--struct nn_thread thread;
local pfd = {}
describe("Poll tests #poll", function()
	-- Test nn_poll() function
	it("creates a pair", function()
		sb = nml.socket(AF_SP, NN_PAIR)
		assert.is_truthy(sb)
		assert.is_not_falsy(sb)
	end)
	it("binds to a socket", function()
		assert.is_not_falsy(nml.bind(sb, SOCKET_ADDRESS))
	end)
	it("creates a second pair", function()
		sc = nml.socket(AF_SP, NN_PAIR)
		assert.is_truthy(sc)
		assert.is_not_falsy(sc)
	end)
	it("connects to the pair", function()
		assert.is_not_falsy(nml.connect(sc, SOCKET_ADDRESS))
	end)

	--print(("sb:[%d], sc:[%d], IN:[%d], out:[%d]"):format(sb, sc, NN_POLLIN, NN_POLLOUT))
	
	it("sb=OUT, sc=OUT (ready to send without blocking)", function()
		-- setup the fds
		pfd[1] = {fd = sb, events = NN_POLLIN|NN_POLLOUT}
		pfd[2] = {fd = sc, events = NN_POLLIN|NN_POLLOUT}
		
		rc, pfd = nml.poll(pfd, 2, 10)
		assert.are_equal(NN_POLLOUT, pfd[1].revents)
		assert.are_equal(NN_POLLOUT, pfd[2].revents)
	end)
	it("sends ABC to sc", function()
		-- sc sends "ABC"
		assert.is_not_falsy(nml.send(sc, "ABC", NN_DONTWAIT))
		nml.sleep(100)
	end)
	it("sb=IN|OUT, sc=OUT", function()
		rc, pfd = nml.poll(pfd, 2, 10)
		--print(("2- poll called. pfd:[%s]"):format(pw(pfd)))
		-- sb can process "ABC"
		assert.are_equal(NN_POLLIN|NN_POLLOUT, pfd[1].revents)
		assert.are_equal(NN_POLLOUT, pfd[2].revents)
	end)
	it("sb recv the message", function()
		msg = nml.recv(sb, NN_DONTWAIT)
		assert.is_truthy(msg)
		assert.are_equal("ABC", msg)
	end)
	it("sb=OUT, sc=OUT #2", function()
		rc, pfd = nml.poll(pfd, 2, 10)
		assert.is_truthy(rc)
		assert.are_equal(NN_POLLOUT, pfd[1].revents)
		assert.are_equal(NN_POLLOUT, pfd[2].revents)
	end)
	
	---------------------------------------------------------
	-- compare result with native file descriptors and select
--[[
	-- sb should be ready to send
	rc = getevents(sb, NN_IN|NN_OUT, 1000)
	assert(rc==NN_OUT)

	-- poll for IN when no msg available, should timeout
	rc = getevents(sb, NN_IN, 10)
	assert(rc==0)

	-- sc sends a msg, and test sb for IN
	nml.send(sc, "ABC", NN_DONTWAIT)
	rc = getevents(sb, NN_IN, 10)
	assert(rc==NN_IN)

	-- sb receives the msg, test sb for NN_IN, should timeout
	rc = getevents(sb, NN_IN, 10)
	assert(rc==0)
end)

--[[
print("*****")
-- /*  Send a message and start polling. This time IN event should be signaled. */

rc = getevents(sc, NN_IN, 1000)
assert(rc == NN_IN)

print("*****")
print("about to receive")

-- /*  Receive the message and make sure that IN is no longer signaled. */
rc, msg = nml.recv(sc, NN_DONTWAIT)
assert(rc~=-1)
assert(msg=="ABC")
rc = getevents (sc, NN_IN, 10)
assert(rc == 0)

print("about to test with threads")

-- /*  Check signalling from a different thread. */
local thread = llthreads.new(routine1, "number:", 1, "nil:", nil, "bool:", true)
assert(thread:start())
rc = getevents (sb, NN_IN, 1000)
assert (rc == NN_IN)
rc, msg = nml.recv(sb, "ABC")
assert(rc~=-1)
assert(msg=="ABC")
thread:join()

-- /*  Check terminating the library from a different thread. */
thread = llthreads.new(routine2, "number:", 2, "nil:", nil, "bool:", true)
assert(thread:start())
rc = getevents (sb, NN_IN, 1000)
assert(rc == NN_IN)
rc, msg = nml.recv(sb, NN_DONTWAIT)
assert (rc < 0 and nml.errno() == ETERM)
thread:join()
]]
-- /*  Clean up. */
	it("closes the sockets", function()
		nml.close(sc)
		nml.close(sb)
	end)
end)