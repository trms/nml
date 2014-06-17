local nml = require'nml._base'
local sub = setmetatable({},  nml)

--[[
TODO: Write a wrapper for sub that:

wraps the send and receive functions to take in topics
make subscribe and unsubscribe store the values in lua, given that nanomsg won't cough them up.
make receive strip the topic, but return it as a second value.
--]]
--this won't work for structures / other message types.
--it will have to be integrated with a serializer / deserializer mechanism, once we
--have such thing.

local function send(self, msg, donotwait)
	error("Cannot send on a subscription topic.")
end


local function recv (self, dontwait)
	local msg, err = nml.recv(self, donotwait)
	-- error("SUB RECV")
	if msg then
		--this socket does not catch the 
		--all case, so look for a matching nul only when all is not there....
		if not self._topics['']  then 
			local topics, recv_topic, parsed_msg = self._topics
			for topic, _ in pairs(topics) do
				recv_topic, parsed_msg = msg:match('^(' .. topic .. ")(.*)$")
				if recv_topic then
					return parsed_msg, recv_topic:sub(1, -2) -- take off the \0
				end
			end
		else
			return msg
		end
	else
		return msg, err
	end

end

--todo: intercept subscribe and unsubscribe

local subscribe ={
	get = function(self, value) --will be nil / error

		return self.topics
	end,
	set = function(self, value)

	 -- nil this is the "all topics" case, which does not strip off the front
		value = value ~= nil and (tostring(value) .. '\0') or ""
		self._topics[value] = true
		return value
	end
}
local unsubscribe =  {
	get = function(self, value) --will be nil / error
	
		return nil, "Cannot get unsubscribe option from a socket. Try `subscribe`."
	end,
	set = function(self, value)
		value = value ~= nil and (tostring(value) .. '\0') or ""
		if self._topics[value] then
			self._topics[value] = nil
			return value
		else
			return nil, ("'%s' is not a topic for this sub socket."):format(value)
		end
	end
}


function sub.sub(args)
	args = args or {}
	if type(args) ~= "table" then
		error(("invalid arguments to nml.sub: Expected nil or a table, received '%s'."):format(type(args)))
	end
	args.protocol = "sub"

	local socket, err = nml._socket(args)
	if not socket then
		return nil, err
	end
	socket._topics = {}
	socket.topics = setmetatable({}, {
		__index = function (t, i)
			--empty set must be explicitly done.
			 if type(i) == "string" then
			 	if i ~= "" and i ~= "\0" then i = i ..'\0' end
			 else
			 	return nil
			 end
			return socket._topics[i]
		end,
		__newindex = function(t, index, value)
			if value then
				socket:setsockopt("subscribe", index)
			else
				socket:setsockopt("unsubscribe", index)
			end
		end,
		__pairs = function(t)
			return next, socket._topics
		end
	})
	socket.recv = recv
	socket.send = send

	socket.options.unsubscribe = unsubscribe
	socket.options.subscribe = subscribe

	return socket
end



return sub