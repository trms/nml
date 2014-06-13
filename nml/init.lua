
--[[-- common socket metheods
TODO:
    
    send
    recv
    poll
    sleep
    
    
    
FINISHED:
	
	setsockopt 
	socket 
    strerror 
    getsockopt
    bind
	connect 
    shutdown
    shutdown_all * shuts down all registered endpoints, local, remote or both (both if no arguments)
    close
    term

NOT IMPORTING:
    errno --wrapped in nml_error
    symbolinfo --in nml.symbols
    symbol --in nml.symbols
    sendmsg --not implemented
    recvmsg --not implemented
    device --not implemented
    cmsg 	--not implemented



--nml specific
options --	field, returns a table of all options when indexed. 
		--	when sets, looks for options and sets the socket to them.

--]]

---[[ Debug stuff
do
	local unpack = unpack or table.unpack
	local pretty = require'pl.pretty'
	local write = pretty.write
	local unpack = unpack or table.unpack
	function ts( ... )
		local ret = {}
		for i = 1 , select("#", ...) do
			local val = (select(i, ...))
			ret[#ret + 1] = type(val) == "table" and write(val ) or tostring(val)
		end
		return unpack(ret)
	end
	function pt(...)
		print(ts(...))
	end
end

--]]

local nml = require'nml.symbols' --symbols and symbol_cat
nml.core = require'nml.core'
nml.option_handlers = require'nml.option_handlers'
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

function nml:bind (url)
	if not self[1] then return nil, self:nml_err("No socket.") end
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
	if not self[1] then return nil, self:nml_err("No socket.") end
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
do
	--if more flags become available, we're going to have to get sophisticated
	--on how we porcess flags.
	local DONTWAIT = nml.symbol_cat.flag.dontwait.value
	function nml:send (msg, dontwait)
		if not self[1] then return nil, self:nml_err("No socket.") end

		local result, err = self.core.send(self[1], msg, dontwait and DONTWAIT or nil)

		if result then 
			return result
		else
			return result, self:nml_error(err)
		end
	end

	function nml:recv (dontwait)
		if not self[1] then return nil, self:nml_err("No socket.") end

		local result, err = self.core.recv(self[1], dontwait and DONTWAIT or nil)

		if result then 
			return result
		else
			return result, self:nml_error(err)
		end

	end
end
function nml:poll ()
end
function nml:sleep ()
end
function nml:shutdown_all(where)
	if not self[1] then return nil, self:nml_err("No socket.") end
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
	if not self[1] then return nil, self:nml_err("No socket.") end
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
	if not self[1] then return nil, self:nml_err("No socket.") end
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
	if not self[1] then return nil, self:nml_err("No socket.") end
	local option, level = self:find_opt(option_name)
	--level will be an error message when opt_val is nil.
	if not option then return option, level end

	local value, err = self.core.getsockopt(self[1], level, option.value, option.type)

	if not value then
			return nil, self:nml_error(err)
	elseif self.option_handlers[option_name] then
		return self.option_handlers[option_name].get(self, value), option.unit_name
	else
		return value, option.unit_name
	end 
end
function nml:setsockopt (option_name, value)
	local success, res
	if not self[1] then return nil, self:nml_err("No socket.") end
	local option, level = self:find_opt(option_name)
	--level will be an error message when opt_val is nil.
	if not option then return option, level end

	if self.option_handlers[option_name] then
		success, res = pcall(self.option_handlers[option_name].set, self, value)
		if not success then
			return nil, ("Error setting option. Value '%s' caused an Lua to error in the option handler: %s"):format(tostring(value), tostring(res))
		else
			value = res
		end
	end
	local value, err = self.core.setsockopt(self[1], level, option.value, value, option.type)
	if value then
		return self
	else
		return nil, self:nml_error(err)
	end 
end

local socket_mt = {}

function socket_mt:__index (index)

	if type(index) == "string" then
		if nml[index] then
			return nml[index]
		elseif self.symbol_cat.socket_option[index] or self.proto_options[index] then
			return self:getsockopt(index)
		
		else
			return nil
		end
	end
end

function socket_mt:__newindex(index, value)
	if type(i) == "string" then
		if socket_option[i] or proto_options and proto_options[i] then
			setter(t, i, v)
		else
			rawset(t, i, v)
		end
	end
end
function nml.socket(args)
	local socket = { 
		local_ep = {}, --local endpoints
		remote_ep = {}, --remote endpoints
	} -- new socket object
	local err
	args = type(args) == "string" and {protocol = args} or type(args) == "table" and args or error("Sockets require a protocol to be specified.")
	assert(args.protocol, "Sockets must have a protocol specified.")
	socket.protocol = assert(nml.symbol_cat.protocol[args.protocol],
		"Sockets must be specified with a defined protocol.")
	socket.domain = args.raw and  nml.symbol_cat.domain.af_sp_raw or  nml.symbol_cat.domain.af_sp
	socket[1], err = nml.core.socket(socket.domain.value , socket.protocol.value)
	if socket[1] < 0 then return nil, self:nml_error(err) end
	--the protocol is the socket definition from nanomsg.
	--protocol.value is the integer that defines the socket and therefore the level.
	socket.proto_options = nml.symbol_cat.transport_option[socket.protocol.name] or {}

	return setmetatable(socket, socket_mt) 
end

setmetatable(nml, {
	__index = function(self, index)
		if self.symbol_cat.protocol[index] then
		--this provides for the short hand of creating a socket by the protocol name. Ex: nml.sub()
			return function(args)
				args = args or {}
				args.protocol = index
				return self.socket(args)
			end
		end
	end
})


---[[local testing
-- pt("domain", symbol_cat.domain, symbol_cat.flag)
--[[Categories:

symbol_cat:	protocol
symbol_cat:	limit
symbol_cat:	transport_option
symbol_cat:	socket_option
symbol_cat:	option_level
symbol_cat:	option_unit
symbol_cat:	flag
symbol_cat:	transport
symbol_cat:	domain
symbol_cat:	error
symbol_cat:	option_type
symbol_cat:	version
--]]
for i, v in pairs(nml.symbol_cat.option_type) do
	-- print('symbol_cat:', i)

end
for i,v in pairs(nml.core) do
	-- print(i, v)
end
local s = nml.sub()
local s2 = nml.pub()


-- print(s.subscribe, s:getsockopt("subscribe"))

-- print(s.linger)
-- s:setsockopt("linger", 500)
-- -- s.subscribe = "test"
-- print(s:setsockopt("subscribe", "test"))
-- -- print(s:getsockopt("subscribe"))

-- local long_name = string.rep("a", 257)
-- print(#long_name)
-- print("Long name", s:setsockopt("subscribe", long_name))
-- print(s:bind("tcp://172.16.200.132:100"))
-- print(s:bind("tcp://172.16.200.132:101"))
-- print(s:bind("inproc://172.16.200.132:101"))
-- print(s:connect("tcp://10.0.0.1:200"))
-- print(s:shutdown_all())
-- print(s:shutdown("tcp://10.0.1:200"))
-- print(s:shutdown("tcp://10.0.0.1:200"))
-- print(s:shutdown("tcp://172.16.200.132:100"))
-- print(s:shutdown("tcp://172.16.200.132:101"))
-- print(s:shutdown("inproc://172.16.200.132:101"))
-- print(s:term())
-- print(s2:getsockopt("linger"))
-- print(s2[1])
-- print(s:close())
-- print("second try", s:close())
-- print(s[1], s2[1])
-- print(nml.strerror(nml.errno()) , s)
-- pt(symbol_cat.transport_option)
-- pt(symbols.NN_NS_OPTION_TYPE, '\n')
local sleep = nml.core.sleep

local s_pub = nml.pub()
local s_sub = nml.sub()
local long_topic = string.rep("a", 1023)
local longer_topic = string.rep("a", 1022)
longer_topic = longer_topic .. "a"
print("Setting subscribe option 1234567890", s_sub:setsockopt("subscribe", long_topic))
print("Setting subscribe option bar", s_sub:setsockopt("subscribe", "bar"))
print("sub socket timeout is set to",  s_sub:getsockopt("rcvtimeo"))
print("setting timeout", s_sub:setsockopt("rcvtimeo", 1000))
print("now set to",  s_sub:getsockopt("rcvtimeo"))
local url = "inproc://pubtest"
s_pub:bind(url)
s_sub:connect(url)
-- print(s_pub[1], s_sub[1])

print("sending on abcdefghij", s_pub:send(longer_topic .."\0hello"))

print("receiving...")
print(s_sub:recv())
-- print("sending world on bar", s_pub:send("bar\0world"))
-- print(s_sub:recv())
-- print("sending world on baz", s_pub:send("baz\0not for u"))
-- print("receiving, but should time out.", s_sub:recv())
-- print("sending world on baz", s_pub:send("\0not for u"))
-- print("receiving, but should time out.", s_sub:recv())
--]]
return nml