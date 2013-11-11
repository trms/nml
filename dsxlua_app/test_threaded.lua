_ENV = require'mediacircus'
local ll = require'llthreads'
local nn = require'nml'
local ts = require'socket'
local  hlthread = {}
local threads = {}



local main = nn.new("pair")

local base_addr = "inproc://lthread_control/"


--------------- THE SHELL CODE
local hlcode = [=[
require'lua_dev_path'
_ENV = require'mediacircus'
local ll = require'llthreads'
local nn = require'nml'
local ts = require'socket'

local thread = nn.new("pair")
local t_address = (...)
--the second argument is the code to load
local t_code = (select(2, ...))

thread:connect(t_address)
local msg = "init"
local state = "ready"

while msg ~= "close" do
	thread:send(state)
	printf('sent: "%s"',state)
	msg = thread:receive()
	printf('received "%s"', msg)
	if msg =="close" then
		state = "closing"
	elseif msg == "do" then
		local code = thread:receive()
		if code and code ~= "" then
			local fun, msg = loadfile(code)
			if not fun then 
				thread:send(sprintf("syntax error:\t'%s'", msg))
			else
				local retval = {pcall(fun)}
				if retval[1] then
					retval[1] = #retval - 1
					--TODO ESCAPE EXISTING \0
					retval = table.concat(retval, "\0")
					thread:send(retval)
				else
					thread:send(sprintf("error executing function:\t'%s'", select(2,table.unpack(retval))))
				end
			end
		end
	end
	--ts.sleep(0)
end
thread:send(state)
return true, "exited normally"

]=]
-------------- END SHELL CODE

function hlthread.new(thread_args, thread_code, ...)
	thread_args = thread_args or {}
	local base_addr = thread_args.base_addr or base_addr
	local eid = #threads + 1
	local thread_address = base_addr .. tostring(eid)
	local main = nn.new("pair")
	main:bind(thread_address)
	threads[eid] = setmetatable({ 
		address = thread_address, 
		eid = eid,
		main = main
	},
	{
		__index = function(t, i)
			if i == "call" then 
				return function(self, code)
					t.main:send("do")
					t.main:send(code)
					return t.main:receive()
				end
			elseif i == "start" then 
				return function(...)
					return t.thread.start(t.thread, select(2,...))
				end
			elseif i == "join" then
				return function(...)
					return t.thread.join(t.thread, select(2,...))
				end

			else
				return	function(...)
					return t.main[i](t.main, select(2,...))
				end
			end
		end
	})
 
	threads[eid].thread  = ll.new(hlcode, threads[eid].address, thread_code, ...)

	return threads[eid]
end

local t = hlthread.new()

printf('t:start(): "%s"', t:start())
--ts.sleep(0)
printf('t:receive(): "%s"', t:receive())
printf('t:send("boof"): "%s"', t:send("boof"))

printf('t:receive(): "%s"', t:receive())


---[[
print(t:call("./thread_sender.lua"))
--
--]]

printf('t:send("close"): "%s"', t:send("close"))
t:join()