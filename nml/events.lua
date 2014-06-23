
local nml = require'nml.symbols' --symbols and symbol_cat
local events =  {
	[nml.symbol_cat.poll.send.value] = "send", send = nml.symbol_cat.poll.send.value,
	[nml.symbol_cat.poll.recv.value] = "recv", recv = nml.symbol_cat.poll.recv.value,
	
}

events[events.send | events.recv] = "both" 
events.both = events.send | events.recv

return events