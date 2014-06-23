--[[---
Nanomsg Binding for Lua
@module Debug

--]]

---[[ nml stuff
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

local nml = require'nml._base'

for i, v in pairs(nml.symbol_cat.protocol) do

	if type(i) == "string" then
		local success, proto = pcall(require, 'nml.protocols.' .. i)
	end
	-- print('symbol_cat:', i)

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

---[[ Local poll testing

-- print('poll', ts(nml.symbol_cat.rcvfd),  nml.sym)




--]]
--[[

local addr = "inproc://a"

local sockets = {[1] = assert(nml.bus():bind(addr))}
local receiver  = assert(nml.bus())
for i = 2, 10 do
	-- print("making socket", i)
	sockets[i] = assert(nml.bus())
	-- print("events = ", sockets[i].events)	
end
for i = 2, 7 do
	-- print("making socket", i)
	sockets[i].connect(sockets[i], addr)
	-- print("events = ", sockets[i].events)	
end

for i = 5,10 do
	-- print(sockets[i][1], sockets[i].core)
	sockets[i].connect(sockets[i], "inproc://b")
end

assert(receiver:connect(addr))

local b_receive = nml.bus():bind("inproc://b")

sockets[6].send(sockets[1], "foo")

local count, ready = nml.poll(sockets)
print(#ready.recv, #ready.send)
for i, v in ipairs(ready.recv) do
	print('revent: ', i, v.fd)
end
--]]
--[[local testing
-- pt("domain", symbol_cat.domain, symbol_cat.flag)
--[=[Categories:

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
--]=]

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

print("Setting subscribe option foo", s_sub:setsockopt("subscribe", "foo"))
print("Setting subscribe option bar", s_sub:setsockopt("subscribe", "bar"))

print("topics:", ts(s_sub.topics))
print("can i index a topic?", s_sub.topics.bar)

print("sub socket timeout is set to",  s_sub:getsockopt("rcvtimeo"))
print("setting timeout", s_sub:setsockopt("rcvtimeo", 1000))
print("now set to",  s_sub:getsockopt("rcvtimeo"))
local url = "inproc://pubtest"
s_pub:bind(url)
s_sub:connect(url)
-- print(s_pub[1], s_sub[1])

print("sending 'on hello on foo'", s_pub:send("hello", "foo"))

print("receiving...", assert(s_sub:recv()))

print("sending world on bar", s_pub:send("world", "bar"))
print(s_sub:recv())
print("sending world on baz", s_pub:send("not for u", "baz"))
print("receiving, but should time out.", s_sub:recv())
print("Setting subscribe option bar", s_sub:setsockopt("subscribe", ""))
print("sending 'nil message' on ''", s_pub:send("nil message", ''))
print("receiving, should work.", s_sub:recv())
print("Setting subscribe option bar", s_sub:setsockopt("unsubscribe", ""))
print("sending world on baz", s_pub:send("not for u", "\0"))
print("receiving, but should time out.", s_sub:recv())
s_sub.subscribe = "baz"
print("Setting subscribe option bar", s_sub._topics['baz\0'] )
s_sub.unsubscribe = "baz"
print("Setting subscribe option bar", s_sub._topics['baz\0'] )
--]]
return nml