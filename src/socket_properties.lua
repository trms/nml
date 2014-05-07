-- socket_properties.lua

struct nml_socket_property_t nml_common_socket_properties []  = {
	{NN_SOL_SOCKET, NN_LINGER,"linger", 0, NML_MILLIS, nml_set_milli_prop_0_true, nml_get_milli_prop_0_true, NML_DEFAULT_LINGER},
	{NN_SOL_SOCKET, NN_SNDBUF, "send_buffer", 0, NML_BYTES, nml_set_buffer, nml_get_buffer},
	{NN_SOL_SOCKET, NN_RCVBUF,"receive_buffer", 0, NML_BYTES, nml_set_buffer, nml_get_buffer},
	{NN_SOL_SOCKET, NN_SNDTIMEO,"send_timeout", 0, NML_MILLIS, nml_set_milli_neg_false, nml_get_milli_neg_false,  NML_DEFAULT_SNDTIMEO},
	{NN_SOL_SOCKET, NN_RCVTIMEO, "receive_timeout", 0, NML_MILLIS, nml_set_milli_neg_false, nml_get_milli_neg_false, NML_DEFAULT_RCVTIMEO},
	{NN_SOL_SOCKET,NN_RECONNECT_IVL,"reconnect_interval", 0, NML_MILLIS, nml_set_milli_prop_0_true, nml_get_milli_prop_0_true,NML_DEFAULT_RECONNECT_IVL},
	{NN_SOL_SOCKET, NN_RECONNECT_IVL_MAX,"reconnect_interval_max", 0, NML_MILLIS, nml_set_milli_neg_false, nml_get_milli_neg_false,NML_DEFAULT_RECONNECT_IVL_MAX},
	{NN_SOL_SOCKET, NN_SNDPRIO,"send_priority",0, NML_PRIORITY, nml_set_priority, nml_get_priority, NML_DEFAULT_SNDPRIO},
	{NN_SOL_SOCKET, NN_IPV4ONLY,"ipv_4_only", 0, NML_CHOICE, nml_set_priority, nml_get_priority,  NML_DEFUALT_IPV4_ONLY},
	{NN_SOL_SOCKET, NN_DOMAIN, "domain", 0, NML_DOMAIN, nml_set_read_only, nml_get_domain,  0},
	{NN_SOL_SOCKET, NN_PROTOCOL,"protocol", 0, NML_PROTOCOL, nml_set_read_only, nml_get_protocol,  0},
	{(int)NULL,(int)NULL,(char *)NULL, (int) NULL, (int)NULL, NULL,NULL, (int)NULL}
	// {NN_SOL_SOCKET, NN_SNDFD, NULL, sizeof(NML_FD_TYPE)},
	// {NN_SOL_SOCKET, NN_RCVFD, NULL, sizeof(NML_FD_TYPE)},
};

local properties = {}

local common = {
}