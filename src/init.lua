---nml
---NanoMSG library for lua
---@classmod nml


---The socket domain 
--@field domain raw or full socket. Set during @{new|construction}



---The protocol that the socket was created with. 
--@field protocol Set during @{new|construction}. 


---The time that a socket tries to send pending outbound messages after close has been called, in milliseconds. 
--@field linger Default value is `1000` (1 second). A false value (`false` or `nil`) means infinite linger.

---Size of the send buffer, in bytes. 
--@field send_buffer To prevent blocking for messages larger than the buffer, exactly one message may be buffered in addition to the data in the send buffer. Default value is 128kB.

---Size of the receive buffer, in bytes. 
--@field receive_buffer To prevent blocking for messages larger than the buffer, exactly one message may be buffered in addition to the data in the receive buffer. Default value is 128kB.

---The timeout for send operation on the socket, in milliseconds. 
--@field send_timeout If message cannot be sent within the specified timeout, `nil` plus a timeout message is returned. False (`false`, but NOT `nil`) means infinite timeout (block forever). Default is false.

---The timeout for receive operation on the socket, in milliseconds. 
--@field receive_timeout If no messages are waiting, then `false` is returned. False (`false`, but NOT `nil`) means infinite timeout (block forever). Default is false

---How long to wait to re-establish a broken connection.
--@field reconnect_interval For connection-based transports such as TCP. Note that actual reconnect interval may be randomised to some extent to prevent severe reconnection storms. Default value is `100` (0.1 second).

--- Specifies maximum reconnection interval. 
--@field reconnect_interval_max This option is to be used only in addition to the @{reconnect_interval} option. On each reconnect attempt, the previous interval is doubled until `reconnect\_interval\_max` is reached. Value of `false` means that no exponential backoff is performed and reconnect interval is based only on @{reconnect_interval}. If `reconnect\_interval\_max` is less than @{reconnect_interval}, it is ignored. Default value is `false`.

---Outbound priority of the socket. 
--@field send_priority This option has no effect on socket types that send messages to all the peers. However, if the socket type sends each message to a single peer (or a limited set of peers), peers with high priority take precedence over peers with low priority. Highest priority is 1, lowest priority is 16. Default value is 8.

---Use only IPv4.
-- --@field ipv_4_only  If set to `false`, both IPv4 and IPv6 addresses are used. Default value is `true`.
local nml = setmetatable({}, {__index = require'nml.core'})

return nml

