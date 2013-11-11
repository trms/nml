-- get the dsx module
local dsx = require'hapi.dsx'

print=dsx.output_debug

local brd = dsx.board[1]

dsx.boards[1]{profile="480i_30m_4x3"}

print(">>type is "..type(dsx.boards[1].clock))

print("clock epoch in seconds: "..dsx.boards[1].clock.epoch.host.." epoch dsx: "..dsx.boards[1].clock.epoch.dsx)

local host_epoch = dsx.boards[1].clock.epoch.host

print("now utc: "..os.time(os.date("!*t")))
print(host_epoch - os.time(os.date("!*t")))
print(host_epoch- os.time())
print(host_epoch- os.time(os.date("*t")))
print(">epoch time as local time: "..os.date("*t", host_epoch).hour..":"..os.date("*t", host_epoch).min..":"..os.date("*t", host_epoch).sec)
print(">delta: "..os.time(os.date("!*t", host_epoch))-os.time(os.date("*t", host_epoch)))

-- the end