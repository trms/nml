local nml = require'nml._base'
local pub = setmetatable({},  nml)

local function send (self, msg, topic, donotwait)
	if topic ~= nil and topic ~= "\0" then
		msg = tostring(topic) ..'\0' .. tostring(msg)
	end

	return nml.send(self, msg, donotwait)
	
end
local function recv(self, msg, donotwait)
	error("nml error: pub sockets do not have a recv method.")
end

function pub.pub(args)
	args = args or {}
	if type(args) ~= "table" then
		error(("invalid arguments to nml.sub: Expected nil or a table, received '%s'."):format(type(args)))
	end

	args.protocol = "pub"
	local socket, err =  nml._socket(args)

	if socket then
		socket.recv = recv
		socket.send = send
	end

	return socket
end

return pub