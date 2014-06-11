
local nml_core = require'nml.core'
local nml = setmetatable({sym={}}, {__index = nml_core})
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
]]

local found, i = true, 0

while found do
	found = nml.symbolinfo(i)
	if found then 
		nml.sym[found.name] = found
	end	
	i=i+1
end

--[[
function nml.new()

end
]]
return nml