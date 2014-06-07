local busted = require("busted")
local nml = require'nml'

local api = {"socket", "close", "setsockopt", "getsockopt", "bind", "connect", "shutdown", "send", "recv", "sendmsg", "recvmsg", "allocmsg", "freemsg", "cmsg", "poll", "errno", "strerror", 
	"symbol", "symbolinfo", "device", "term", 
	socket="socket", close="close", setsockopt="setsockopt", getsockopt="getsockopt", bind="bind", connect="connect", shutdown="shutdown", send="send", recv="recv", sendmsg="sendmsg", recvmsg="recvmsg", allocmsg="allocmsg", freemsg="freemsg", cmsg="cmsg", poll="poll", errno="errno", strerror="strerror", 
	symbol="symbol", symbolinfo="symbolinfo", device="device", term="term"}

local protocols = {"pair", "reqrep", "pubsub", "survey", "pipeline", "bus",
	pair="pair", reqrep="reqrep", pubsub="pubsub", survey="survey", pipeline="pipeline", bus="bus"}

describe "testing nml creation #creation" (function()
	it "is not nil" (function()
		assert.is_truthy(nml)
	end)
	
	it "is a table" (function()
		assert.are_same("table", type(nml))
	end)
end)

describe "supports the expected api functions #api" (function()
	for i=1, #api do
		it ("supports the "..api[i].." api", function()
			assert.is_truthy(nml[api[i]])
			assert.are_equal("function", type(nml[api[i]]))
		end)
	end
end)