local nml_symbols = require'nml.symbols'
local symbol_cat = assert(nml_symbols.symbol_cat)
nml_symbols = nil
_ENV = nil
local options = {}

local trans_get = function (value, cat)
	return cat[value] and cat[value].name
end

local trans_set = function (name,cat)
	return cat[name] and cat[name].value
end

options.protocol = {
get = function (self, value)
	return trans_get(value, symbol_cat.protocol)
end,
set = function (self, name)
	return trans_set(value, symbol_cat.protocol)
end}

options.domain = {
	get = function (self, value)
		return trans_get(value, symbol_cat.domain)
	end,
	set = function (self, name)
		return trans_set(value, symbol_cat.domain)
	end
}

options.ipv4only = {
	get = function (self, value)
		if value ~= nil then 
			return value and value == 1 
		else
			return nil
		end
	end,
	set = function (self, value)
		return value and 1 or 0
	end
}

options.sndtimeo = {
	get = function (self, value)
		if value ~= nil then 
			return value > -1 and value or false
		else
			return nil
		end
	end,
	set = function (self, value)
		return value or -1
	end
}


options.rcvtimeo = {
	get = function (self, value)
		if value ~= nil then 
			return value > -1 and value or false
		else
			return nil
		end
	end,
	set = function (self, value)
		return value or -1
	end
}

return options