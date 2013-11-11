_ENV = require'mediacircus'


n = require'nml'



echo = n.new"pair"

URL = "tcp://10.0.74.54:5000"
echo:connect(URL)
msg = ""
while msg ~= "close" do
	print("start loop")
	msg = echo:receive()
	print("test_receive got: ", msg)
	-- if msg ~= "close" then
	-- 	echo:send(msg)
	-- end

end

print("goodbye!")