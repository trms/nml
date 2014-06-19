--[[---
@submodule nml
--]]

local nml = require'nml.symbols' --symbols and symbol_cat
nml.core = require'nml.core'
nml.options = require'nml.options'
nml.sym = nml.symbols

local def_option_level = assert(nml.symbol_cat.option_level.sol_socket.value, "There is a default socket level.")

do
	local errno, strerror = assert(nml.core.errno, "There is an errno function in nml.core"), assert(nml.core.strerror, "There is an strerr function in nml.core.")
	local ETERM = nml.symbol_cat.error.eterm.value
	function nml:nml_error(n)
		n = n or errno()
		if type(n) == "string" then
			return n
		elseif n ~= 0 then
			if n == ETERM and self[1] then
				self:close()
			end

			return ("nanomsg error[%d]: %s"):format(n, strerror(n)), n
		else
			return nil
		end
	end
end

local socket_mt = {}


function socket_mt:__index (index)

	if type(index) == "string" then
		if nml[index] then
			return nml[index]
		elseif self.symbol_cat.socket_option[index] or self.proto_options[index] then
			return self:getsockopt(index)
		elseif index == "events" then
			if self._events == nil then
				return events.send | events.recv
			else
				return self._events
			end
		elseif self.core[index] then
			return self.core[index]
		else
			return nil
		end
	end
end

function socket_mt:__newindex(index, value)

	if self.symbol_cat.socket_option[index] or self.proto_options and self.proto_options[index] then
			self:setsockopt(index, value)
	elseif index == "events" then
		if value =="send" then
			rawset(self, "_events", _self.sybmol_cat.poll.send)
		elseif value == "recv" then
			rawset(self, "_events", _self.sybmol_cat.poll.recv)
		elseif value == "both" then
			rawset(self, "_events", 
				_self.sybmol_cat.poll.recv |  _self.sybmol_cat.poll.send
			)
		elseif not value then
			rawset(self, "_events", 0)
		elseif type(value) == "number" then
			rawset(self, "_events", value)
		else
 			error(("Unknown event: %d"): format(tostring(value)))
		end
	else
		rawset(self, index, value)
	end
end

--[[---
Creates a new nanomsg scalability protocol (SP) socket.
@param args A table of the arguments
@param args.protocol The name of the scalability protocol
@param args.raw IF raw, then an AF_SP_RAW socket is created. If ommited or otherwise false, a full socket is created.
@return An nml socket
--]]
function nml.socket(args)
	args = type(args) == "string" and {protocol = args} or 
		type(args) == "table" and args or 
		error("Sockets require a protocol to be specified.")

	if rawget(nml, args.protocol) then
		return nml[args.protocol](args)
	else
		return nml._socket(args)
	end
end
function nml._socket(args)

	local socket = { 
		local_ep = {},	--local endpoints
		remote_ep = {}, --remote endpoints
	} -- new socket object
	local err
	
	assert(args.protocol, "Sockets must have a protocol specified.")
	--if the specific protocol has a constructor, call it, instead.
	nml.flag = nml.symbol_cat.flag
	socket.protocol = assert(nml.symbol_cat.protocol[args.protocol],
		"Sockets must be specified with a defined protocol.")
	socket.domain = args.raw and  nml.symbol_cat.domain.af_sp_raw or  nml.symbol_cat.domain.af_sp
	socket[1], err = nml.core.socket(socket.domain.value , socket.protocol.value)
	socket.fd = socket[1] -- do this so we can pass in poll

	if socket[1] < 0 then return nil, self:nml_error(err) end
	--the protocol is the socket definition from nanomsg.
	--protocol.value is the integer that defines the socket and therefore the level.
	socket.proto_options = nml.symbol_cat.transport_option[socket.protocol.name] or {}
	setmetatable(socket, socket_mt) 

	return socket
end


function nml:bind (url)
	if not self[1] then return nil, self:nml_error("No socket.") end
	if not url then
		return nil, "URL required for binding."
	end
	url = tostring(url)
	local ep, err = self.core.bind(self[1], tostring(url))
	if ep ~= -1 then
		self.local_ep[url] = ep
		self.local_ep[ep] = url
		return self
	else
		return nil, self:nml_error(err)
	end
end

function nml:connect (url)

	if not self[1] then return nil, self:nml_error("No socket.") end
	if not url then
		return nil, "URL required for connecting."
	end
	url = tostring(url)
	local ep, err = self.core.connect(self[1], url)
	if ep ~= -1 then
		self.remote_ep[url] = ep
		self.remote_ep[ep] = url
		return self
	else
		return nil, self:nml_error(err)
	end
end

--[[
Note:
we may want to add the ability to process messages using a serializer/deserializer. This way,
we could provide a documented way to write a C handler for messages and we'd be able to use
zero copy to do it.
--]]
local events = {
	[nml.symbol_cat.poll.send.value] = "send", send = nml.symbol_cat.poll.send.value,
	[nml.symbol_cat.poll.recv.value] = "recv", recv = nml.symbol_cat.poll.recv.value,
	
}
events[events.send | events.recv] = "both" 
events.both = events.send | events.recv

function nml:send (msg, dontwait)
	if not self[1] then return nil, self:nml_error("No socket.") end
	local result, err = self.core.send(self[1], msg, dontwait and self.flag.dontwait.value or nil)

	if result then 
		return result
	else
		return result, self:nml_error(err)
	end
end

function nml:recv (dontwait)
	if not self[1] then return nil, self:nml_error("No socket.") end

	local result, err = self.core.recv(self[1], dontwait and self.flag.dontwait or nil)

	if result then 
		return result
	else
		return result, self:nml_error(err)
	end

end

function nml.poll (...)
	local timeout
	local fds, count, ready
	
	if  select("#", ...) > 1 then
		fds = { ... }
	elseif type(...) == "table" then --assume a sequence was passed in first 
		fds = (...)
	else
		error(("nml error: Expected a sequence of sockets to poll. Received '%s'"):format(type((...))) )
	end

	if type(fds[#fds]) == "number" then
		timeout =  fds[#fds] 
		fds[#fds] = nil
	else
		timeout = 0
	end
	if #fds > 0 then

		count, fds = nml.core.poll(fds,#fds, timeout)

		if count > 0 then
			ready = {send = {}, recv = {}}

			for i, s in ipairs(fds) do

				if (events.send & s.revents) > 0 then
					ready.send[#ready.send + 1] = s
				end
				if (events.recv & s.revents) > 0 then
					ready.recv[#ready.recv + 1] = s
				end
				s.revents = nil
			end
			return ready
		end
	end

end

function nml:shutdown_all(where)
	if not self[1] then return nil, self:nml_error("No socket.") end
	if not where then 
		self:shutdown_all("local_ep")
		return self:shutdown_all("remote_ep")
	end
	for i, v in pairs(self[where]) do
		if type(i) == "string" then
			self:shutdown(i)
		end
	end
	return self
end
function nml:shutdown (url)
	if not self[1] then return nil, self:nml_error("No socket.") end
	if not url then return nil, "nml error: url required for shutdown." end
	url = tostring(url) --do this for url-like objects
	local ep, success, err
	local where = self.local_ep[url]  and "local_ep" or self.remote_ep[url] and "remote_ep" or nil
	if where then
		ep = self[where][url]
		success, err = self.core.shutdown(self[1], ep)
		if success then
			self[where][ep] = nil
			self[where][url] = nil
			return self
		else
			return success or self:nml_error(err)
		end
	else
		return nil, ("nml_error: endpoint '%s' not found"):format(url)
	end


end
function nml:close ()
	if not self[1] then return nil, self:nml_error("No socket.") end
	local result, err =  self.core.close(self[1])
	if result then
		self[1] = nil
		self.local_ep = {}
		self.remote_ep = {}
		return self
	else
		return nil, self:nml_error(err)
	end
end
function nml:term ()
	self.core.term()
	--term is library wide, so we do not need to have a socket in order to call it.
	--therefore, only close if we have a socket.
	if self[1] then
		return self:close()
	else
		return true
	end
end

function nml:find_opt(option_name)
	local option, level
	if self.symbol_cat.socket_option[option_name] then
		option = self.symbol_cat.socket_option[option_name]
		level = def_option_level
	elseif self.proto_options[option_name] then
		option = self.proto_options[option_name]
		level = self.protocol.value
	else
		return nil, ("No such options '%s'."):format(tostring(option_name))
	end
	return option, level
end
function nml:getsockopt (option_name)
	if not self[1] then return nil, self:nml_error("No socket.") end
	local option, level = self:find_opt(option_name)
	--level will be an error message when opt_val is nil.
	if not option then return option, level end

	local value, err = self.core.getsockopt(self[1], level, option.value, option.type)
	--do this, even for errors.
	if self.options[option_name] and self.options[option_name].get then
		value, err =  self.options[option_name].get(self, value)
	end

	if value == nil then
			return nil, self:nml_error(err)
	else
		return value, option.unit_name
	end 
end
function nml:setsockopt (option_name, value)
	local success, res, err
	if not self[1] then return nil, self:nml_error("No socket.") end
	local option, level = self:find_opt(option_name)
	--level will be an error message when opt_val is nil.
	if not option then return option, level end

	if self.options[option_name] and self.options[option_name].set then
		success, res, err = pcall(self.options[option_name].set, self, value)
		if not success or res == nil and err then
			 err = success and err or res
			return nil, tostring(err)
		else
			value = res
		end
	end
	value, err = self.core.setsockopt(self[1], level, option.value, value, option.type)
	if value then
		return self
	else
		return nil, self:nml_error(err)
	end 
end


return nml