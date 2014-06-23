local core = require'nml.core'
local errno, strerror = assert(core.errno, "There is an errno function in nml.core"), assert(core.strerror, "There is an strerr function in nml.core.")
local sym = require'nml.symbols' --symbols and symbol_cat
local ETERM = sym.symbol_cat.error.eterm.value
return function (self, n)
	if type(self) ~= "table" then
		n = self
	end
	
	n = n or errno()
	if type(n) == "string" then
		return n
	elseif n ~= 0 then
		if n == ETERM and self[1] then
			self:close()
		end

		return ("nanomsg error[%d]: %s"):format(n, strerror(n)), n
	else
		return nil
	end
end