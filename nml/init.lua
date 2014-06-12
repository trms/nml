local nml_core = require'nml.core'

local unpack = unpack or table.unpack
local symbols, symbol_cat = unpack(require'nml.config')
local errno, strerr = nml_core.errno, nml_core.strerr

local nml_strerr = function(n)
	n = n or errno()
	if n ~= 0 then
		return ("%d: %s"):format(n, strerr(n))
	else
		return false
	end
end


---[[ Debug stuff
do
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
local option_handlers = require'nml.option_handlers'

local socket_option = symbol_cat.socket_option
local transport_option = symbol_cat.transport_option
local option_type = symbol_cat.option_type

local def_option_level = assert(symbol_cat.option_level.sol_socket.value, "There is a default socket level")
local getsockopt = assert(nml_core.getsockopt, "There is a getsockopt function.")

local function find_opt(option_name, socket_type, t_opts)
	local option, value, level, option_type
	if socket_option[option_name] then
		option = socket_option[option_name]
		level = def_option_level
	elseif  t_opts and t_opts[option_name] then
		option = t_opts[option_name]
		level = socket_type
	else
		return nil, ("No such options '%s'."):format(tostring(option_name))
	end
	value = option.value
	option_type = option.type

	return value, level, option_type
end
local options_factory = function(socket, socket_type, t_opts)

	socket_type = socket_type or def_option_level
	socket = assert(socket, "Factory has a socket.")

	local function getter (option_name)

		local opt_val, level, option_type = find_opt(option_name, socket_type, t_opts)
		--optval will be an error message.
		if not level then return level, opt_val end

		local val = getsocketopt(socket, level, opt_val, option_type)
		if not val then
			return nil, nml_strerr()
		elseif option_handlers[option_name] then
			return option_handlers[option_name] (val)
		else
			return val
		end 


	end



end

local sp_constructors = {}
do
	local domains = symbol_cat.domain
	local socket = nml_core.socket
	local transport_option = symbol_cat.transport_option
	local socket_option = symbol_cat.socket_option
	for i, v in pairs(symbol_cat.protocol) do
		if type(i) == "string" then
			local proto = v.value
			sp_constructors[i] = function(args)
				local args = args or {}
				local domain = args.raw and  domains.af_sp_raw.value or  domains.af_sp.value
				local s = socket(domain , proto)
				if s < 0 then return nil, nml_strerr() end
				local obj = {s}
				--TODO SET SOCKET OPTIONS
				pt(v.name, transport_option[i])


				return obj
			end
		end
	end
end

-- pt(symbol_cat.domain)
local nml = setmetatable({sym = symbols}, {__index = nml_core})


function nml.new (args)

	local raw = args.raw 
end



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
for i, v in pairs(symbol_cat.option_type) do
	print('symbol_cat:', i, ts(v))

end
for i,v in pairs(nml_core) do
	-- print(i, v)
end
local s = sp_constructors.sub()
local s2 = sp_constructors.pub()
-- print(s[1], s2[1])
-- print(nml.strerror(nml.errno()) , s)
-- pt(symbol_cat.transport_option)
-- pt(symbols.NN_NS_OPTION_TYPE, '\n')

--]]
return setmetatable(nml, {__index = nml_core})