#include "nml.h"
/*

chunk manipulation functions

example of a nml message format:

0       4       8                                                         104     108     112       128
+-------+-------+----------------------------------------------------------+-------+-------+---------+ 
|       |       |                                                          |       |       |         | 
|   M   |       |                                                          |   J   |       |         | 
|   S   | 0x0060|                                                          |   U   | 0x0010| YYYYYYY | 
|   G   |       |                         data                             |   N   |       |         | 
|   P   |       |                                                          |   K   |       |         | 
|       |       |                                                          |       |       |         | 
+-------+-------+----------------------------------------------------------+-------+-------+---------+ 
*/

// in bytes
static const int g_iHeaderOffset = 0;
static const int g_iSizeOffset = 4;
static const int g_iDataOffset = 8;

void* ck_alloc(const int in_i)
{
	// data + ckid + cksize
	void* pv = nn_allocmsg(in_i+g_iDataOffset, 0);

	// store the size
	*((uint32_t*)pv+(g_iSizeOffset>>2)) = in_i;

	return pv;
}

int ck_free(void* in_pv)
{
	return nn_freemsg(in_pv);
}

void* ck_realloc(void* in_pv, const int in_i)
{
	return nn_reallocmsg(in_pv, in_i+g_iDataOffset);
}

void* ck_get_data(void* in_pvck)
{
	return (uint8_t*)in_pvck+g_iDataOffset;
}

void ck_set_header(void* in_pvck, const DWORD in_dw)
{
	*((uint32_t*)in_pvck+(g_iHeaderOffset>>2)) = in_dw;
}

void ck_copy_data(void* io_pvck, const void* in_pvsrc, const size_t in_iSize)
{
	memcpy((uint8_t*)io_pvck+g_iDataOffset, in_pvsrc, in_iSize);
}

const char* ck_get_header(void* in_pvck)
{
	return (char*)in_pvck+g_iHeaderOffset;
}

int ck_get_size(void* in_pvck)
{
	return *((uint8_t*)in_pvck+g_iSizeOffset);
}