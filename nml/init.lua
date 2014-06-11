local nml_core = require'nml.core'

local unpack = unpack or table.unpack
local symbols, symbol_cat = unpack(require'nml.config')

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



local function nml_index( t, i)
	if type(i) ~= "string" then
		return nil
	end

	if symbol_cat.protocol[i] then

		do
			local protocol = symbol_cat.protocol[i]
			local socket = nml_core.socket
			local domains = symbol_cat.domain
			
			local print = print
			local _ENV = nil
			return function (args)
				local domain = domains[args and args.raw and "af_sp_raw" or "af_sp"]
				print('going with', domain.value, domain.name, protocol.value, protocol.name)
				return socket(domain.value, protocol.value)
			end
		end
	end
end

local nml = setmetatable({}, {__index = nml_core})


function nml.new (args)

	local raw = args.raw 
end





---[[local testing
-- pt("domain", symbol_cat.domain, symbol_cat.flag)
for i, v in pairs(symbol_cat.domain) do
	-- print('symbol_cat:', i)

end
for i,v in pairs(nml_core) do
	-- print(i, v)
end
local s = nml_index(t, "sub")()
print(s)
print(nml.strerror(), s)
-- pt(symbol_cat.transport_option)
-- pt(symbols.NN_NS_OPTION_TYPE, '\n')

--]]
return nml