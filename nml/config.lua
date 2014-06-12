local nml_core = require'nml.core'
local symbols, symbol_cat = {}, {}

local lower = string.lower
local type, error, assert, pairs, print, tostring = type, error, assert, pairs, print, tostring
local ts, pt = ts or tostring, pt or print --for debug output, if present
local found, i = true, 0
local types = {NN_TYPE_STR = "string", NN_TYPE_INT = "number"}
local transport

local ns = {}

_ENV = nil

while found do
	found = nml_core.symbolinfo(i)
	if found and found.ns == 0 then
		assert(ns[found.value] ==nil,("ns already found: %s\n%s"):format( ts(ns[found.value]), ts(found) ) )
		ns[found.value] = found
		local name = found.name
		local lua_name = name:lower():match("^nn_ns_(.*)$") or  name == "NN_TYPE_NONE" and true or
			error(("Failed to match namespace %s (%d) in %s: \n%s\n%s"):format(name, found.ns, lua_name, ts(found), ts(symbols[found.ns]) ))
		found.c_name = found.name
		found.name  = lua_name
		symbol_cat[lua_name] = lua_name ~= "namespace" and {} or nil

	end
	i=i+1
end

i = 0
local c_name_space, value, name_space, target_table, lua_name
repeat
	found = nml_core.symbolinfo(i)
	if not found or found.ns == 0 then
		goto continue
	end
	lua_name = types[found.name] or found.name:lower():match("^nn_(.*)$") or found.name:lower()
	-- print('lua_name', lua_name)
	if ns[found.ns] then
		c_name_space = ns[found.ns].c_name
		name_space = ns[found.ns].name
	else
		error(("No namespace for %s"):format(found.name))
	end
	value = found.value

	if lua_name:match("^unit_") then
		lua_name = lua_name:match("^unit_(.*)$")
	end

	target_table = symbol_cat[name_space]
	if name_space == "transport_option" then
		-- print("transport_option name space:", lua_name, lua_name:match("^(%w+)_(.*)$"))
		transport, lua_name = assert(lua_name:match("^(%w+)_(.*)$"))
		target_table[transport] = target_table[transport] or {}
		target_table = target_table[transport]
	end
	found.c_name_space = c_name_space
	found.name_space = name_space
	found.c_name = found.name
	found.name = lua_name
	found.ns = nil

	target_table[lua_name] = found
	target_table[value] = found

	symbols[found.c_name] = found

	::continue::
	i = i + 1
	
until not found 


local map_name = function(item,field,  categories)
	item[field .. "_name"] = item[field] > 0 and categories[item[field]].name or nil
	return item
end

for i, v in pairs(symbols) do
	if type(i) == "string" then 
		map_name(v, "unit", symbol_cat.option_unit)
		map_name(v, "type", symbol_cat.option_type)

	end
end


return {symbols, symbol_cat}