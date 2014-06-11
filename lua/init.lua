local nml_core = require'nml.core'
local nml = setmetatable({sym={}}, {__index = nml_core})

--[[
local pretty = require'pl.pretty'
local write = pretty.write
local unpack = unpack or table.unpack
local lower = string.lower
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

local function nn_name (str)
	return str:lower():match("^nn_(.*)$") or str:lower()
end
local function split_ns(str)
	local ns = {}
	local level = ns
	for name in str:gmatch("(%w+)") do
		if name ~= "ns" then 
			level[name] = {}
			level = level[name]
		-- print(name)
		end
	end

	return ns
end
-- split_ns("ns_namespace")

local symbols, symbols_cat, found, i = {}, {}, true, 0
local f= nil

local types = {NN_TYPE_INT = "number", NN_TYPE_NONE = "nil", NN_TYPE_STR = "string"}
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
	f = nml.symbolinfo(i)
	assert(f == nil or found ~= f)
	if found then
		local name_space = nn_name(nml.symbolinfo(found.ns).name):match("^ns_(.*)$") or error(("failed to match namespace %s"):format(nml.symbolinfo(found.ns).name))
		-- print('name_space', name_space)
		local name = nn_name(found.name)
		if name:match("ns_.*$") then
			name = name:match("^ns_(.*)$") 
			-- print("found name_space", name_space, name, i)
		elseif name:match("unit_.*$") then
			print('found unit', name)
		end

		found.name_space = name_space
		found.value = tonumber(found.value) and (found.value //1)
		found.lua_name = nn_name(name)
		-- print(found.lua_name, found.name, name_space)
		symbols[found.value] = found
		symbols[found.lua_name] = found

		symbols_cat[name_space] = symbols_cat[name_space] or {}
		symbols_cat[name_space][found.lua_name] = found

	
	end
	i=i+1
end 
for i, v in pairs(symbols) do

	if v.ns == 9 then
		-- pt(v)
		-- print(v.lua_name, v.value)
		-- pt(split_ns(v.lua_name))
	end
end

-- pt(symbols.NN_NS_OPTION_TYPE, '\n')
-- pt(symbols_cat)
-- print(symbols_cat == symbols, symbols_cat.NN_NS_OPTION_TYPE == symbols.NN_NS_OPTION_TYPE)
]]
return nml