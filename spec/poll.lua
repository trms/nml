--local llthreads = require'llthreads'
local nml=require'nml'
--local pw=require'pl.pretty'.write
require'busted'

local PAIR_ADDR = "inproc://pair"
local msg
local count
local sockets 
local pair_1
local pair_2
describe("Poll tests #poll", function()
	-- Test nn_poll() function
	it("creates a pair", function()
		pair_1 = nml.pair()
		assert.is_truthy(pair_1)
		assert.is_not_falsy(pair_1)
	end)
	it("binds to a socket", function()
		assert.is_not_falsy(nml.bind(pair_1, PAIR_ADDR))
	end)
	it("creates a second pair", function()
		pair_2 = nml.pair()
		assert.is_truthy(pair_2)
		assert.is_not_falsy(pair_2)
	end)
	it("connects to the pair", function()

		assert.is_not_falsy(nml.connect(pair_2, PAIR_ADDR))
	end)

	--print(("pair_1:[%d], pair_2:[%d], IN:[%d], out:[%d]"):format(pair_1, pair_2, NN_POLLIN, NN_POLLOUT))
	
	it("pair_1=OUT, pair_2=OUT (ready to send without blocking)", function()
		-- setup the fds
		sockets = { 
			pair_1, 	--{fd = pair_1, events = NN_POLLIN|NN_POLLOUT}
			pair_2,			 --{fd = pair_2, events = NN_POLLIN|NN_POLLOUT}
		}
		
		count, sockets = nml.poll(sockets, 10)
		assert.are_equal(NN_POLLOUT, sockets[1].revents)
		assert.are_equal(NN_POLLOUT, sockets[2].revents)
	end)
	it("sends ABC to pair_2", function()
		-- pair_2 sends "ABC"
		assert.is_not_falsy(nml.send(pair_2, "ABC", NN_DONTWAIT))
		nml.sleep(100)
	end)
	it("pair_1=IN|OUT, pair_2=OUT", function()
		count, pfd = nml.poll(pfd, 2, 10)
		--print(("2- poll called. pfd:[%s]"):format(pw(pfd)))
		-- pair_1 can process "ABC"
		assert.are_equal(NN_POLLIN|NN_POLLOUT, pfd[1].revents)
		assert.are_equal(NN_POLLOUT, pfd[2].revents)
	end)
	it("pair_1 recv the message", function()
		msg = nml.recv(pair_1, NN_DONTWAIT)
		assert.is_truthy(msg)
		assert.are_equal("ABC", msg)
	end)
	it("pair_1=OUT, pair_2=OUT #2", function()
		count, pfd = nml.poll(pfd, 2, 10)
		assert.is_truthy(count)
		assert.are_equal(NN_POLLOUT, pfd[1].revents)
		assert.are_equal(NN_POLLOUT, pfd[2].revents)
	end)
	
-- /*  Clean up. */
	it("closes the sockets", function()
		nml.close(pair_2)
		nml.close(pair_1)
	end)
end)