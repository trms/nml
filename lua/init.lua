
local nml_core = require'nml.core'
local nml = setmetatable({}, {__index = nml_core})
--[[
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



local symbols, found, i = {}, true, 0

while found do
	found = nml.symbolinfo(i)
	-- pt(found)
	if found and symbols[found.name] then 
		symbols[found.name] = found
	end
	
	i=i+1
end 

pt(symbols.NN_DOMAIN)
function nml.new()

end

--]]




return nml