#include "nml.h"

// query the names and properties of nanomsg symbols
// http://nanomsg.org/v0.3/nn_symbol_info.3.html
int l_symbolinfo(lua_State* L)
{
	return pushError(L, _T("not implemented"));
}