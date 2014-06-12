_ENV = nil
local option_handlers = {}

local trans_get = function (value, cat)
	return cat[value] and cat[value].name
end

local trans_set = function (name,cat)
	return cat[name] and cat[name].value
end

option_handlers.protocol = {
get = function (value)
	return trans_get(value, symbol_cat.protocol)
end,
set = function (name)
	return trans_set(value, symbol_cat.protocol)
end}

option_handlers.domain = {
	get = function (value)
		return trans_get(value, symbol_cat.domain)
	end,
	set = function (name)
		return trans_set(value, symbol_cat.domain)
	end
}
option_handlers.ipv4only = {
	get = function (value)
		return value == 1 
	end,
	set = function (value)
		return value and 1 or 0
	end
}

option_handlers.sndtimeo = {
	get = function (value)
		return value > -1 and value or false
	end,
	set = function (value)
		return value or -1
	end
}


option_handlers.rcvtimeo = {
	get = function (value)
		return value > -1 and value or false
	end,
	set = function (value)
		return value or -1
	end
}

return option_handlers