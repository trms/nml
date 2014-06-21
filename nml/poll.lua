local events = require'nml.events'
local poll = require'nml.core'.poll

local nml = {}

function nml:poll (sockets, timeout)
	local timeout

	--trap if called with colon.
	if type(sockets) ~= "table" then --assume a sequence was passed in first 
		sockets, timeout = self, sockets
	end

	if type(sockets) ~= "table" then
		error(("nml error: Expected a sequence of sockets to poll. Received '%s'"):format(type((sockets))) )
	else
		if sockets.send then
			for i, v in ipairs(sockets.send) do sockets.send[i] = nil end
		else
			sockets.send = {}
		end
		if sockets.recv then
			for i, v in ipairs(sockets.recv) do sockets.recv[i] = nil end
		else
			sockets.recv = {}
		end
		
	end

	timeout = timeout or 0

	local count, err, res

	if #sockets > 0 then
		count, res = poll(sockets, timeout)
		if not count then return count, sockets, res end --res will be error message
		assert(res == sockets, "The table is recycled.")
		if count > 0 then
			for i, s in ipairs(sockets) do
				if (events.send & s.revents) > 0 then
					sockets.send[#sockets.send + 1] = s
				end
				if (events.recv & s.revents) > 0 then
					sockets.recv[#sockets.recv + 1] = s
				end
		
				-- s.revents = nil
			end
			
		end
	else
		count = 0
		err = "No sockets received. Expected sequence of nml sockets."
	end

	return count, sockets, err

end

return nml