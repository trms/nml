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


local sp_constructors = {}
do
	local domains = symbol_cat.domain
	local socket = nml_core.socket
	local transport_option = symbol_cat.transport_option
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
				-- pt(v.name, transport_option[i])


				return obj
			end
		end
	end
end

-- pt(symbol_cat.domain)
local nml = setmetatable({sym = assert(symbols)}, {__index = nml_core})


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
local s = sp_constructors.sub()
local s2 = sp_constructors.pub()
-- print(s[1], s2[1])
-- print(nml.strerror(nml.errno()) , s)
-- pt(symbol_cat.transport_option)
-- pt(symbols.NN_NS_OPTION_TYPE, '\n')

--]]
return setmetatable(nml, {__index = nml_core})