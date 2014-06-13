local nml_symbols = require'nml.symbols'
local symbol_cat = assert(nml_symbols.symbol_cat)
nml_symbols = nil
_ENV = nil
local option_handlers = {}

local trans_get = function (value, cat)
	return cat[value] and cat[value].name
end

local trans_set = function (name,cat)
	return cat[name] and cat[name].value
end

option_handlers.protocol = {
get = function (self, value)
	return trans_get(value, symbol_cat.protocol)
end,
set = function (self, name)
	return trans_set(value, symbol_cat.protocol)
end}

option_handlers.domain = {
	get = function (self, value)
		return trans_get(value, symbol_cat.domain)
	end,
	set = function (self, name)
		return trans_set(value, symbol_cat.domain)
	end
}
option_handlers.ipv4only = {
	get = function (self, value)
		return value == 1 
	end,
	set = function (self, value)
		return value and 1 or 0
	end
}

option_handlers.sndtimeo = {
	get = function (self, value)
		return value > -1 and value or false
	end,
	set = function (self, value)
		return value or -1
	end
}


option_handlers.rcvtimeo = {
	get = function (self, value)
		return value > -1 and value or false
	end,
	set = function (self, value)
		return value or -1
	end
}

return option_handlers