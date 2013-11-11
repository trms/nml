_ENV = require'mediacircus'

n = require'nml'
msg_str = "hello!"


echo = n.new"pair"

URL = "tcp://*:5000"
echo:bind(URL)
msg = ""

for i = 1, 500 do
	msg = echo:send(msg_str .."\t" .. tostring(i))

	print("test send got: ", msg)
end
echo:send("close")


