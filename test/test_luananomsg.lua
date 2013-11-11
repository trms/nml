require'lua_dev_path'
print = dsx.output_debug
print("hello!")
_ENV = require'mediacircus'

n = require'nml'

s = n.new("pair")
s2 = n.new("pair")

s:bind("inproc://test")
s:bind("tcp://*:5560")

s2:connect("inproc://test")

s:send("test")

print(s2:receive())

pt(s.end_points)

print("s2:")
pt(s.end_points)
s:shutdown(2)
pt(s.end_points)
s:bind("tcp://*:5561")
pt(s.end_points)
s:shutdown(true)
pt(s.end_points)
--s:close()
s = nil

--collectgarbage()

--[[
print("linger is default", s.linger)
-- print("send_buffer is:", s.send_buffer / 1024)



s.linger = true
print("linger is true:", s.linger)
s.linger = nil
print("linger is nil:", s.linger)
s.linger = 99.9
print("linger is 99.9", s.linger)
local success, value =  pcall(function() s.linger = "potatoes" end)
print("success, value", success, value)
print("linger is potatoes", s.linger)
print("NOW TESTING BUFFER:\n")

print("send_buffer is:", s.send_buffer / 1024)
s.send_buffer = 1024 *1024
print("send_buffer 1024 * 1024", s.send_buffer / 1024)
print("receive_buffer is:", s.receive_buffer / 1024)
s.receive_buffer = 2048 *1024
print("receive_buffer 2048 * 1024", s.receive_buffer / 1024)

print("s.domain", s.domain)
--print("s.ipv_4_only", s.ipv_4_only)
--s.ipv_4_only = false
--print("s.ipv_4_only", s.ipv_4_only)
 --s.domian = "fart"
print("s.domin", s.domian)
print("s.protocol", s.protocol)

--n.get_symbols()

--]]