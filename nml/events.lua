
local nml = require'nml.symbols' --symbols and symbol_cat
return {
	[nml.symbol_cat.poll.send.value] = "send", send = nml.symbol_cat.poll.send.value,
	[nml.symbol_cat.poll.recv.value] = "recv", recv = nml.symbol_cat.poll.recv.value,
	
}