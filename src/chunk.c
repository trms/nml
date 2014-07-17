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

Any void* coming from the outside is considered to pointing to 'data', so must be offset internally when accessing header and size, as well as realloc and free.

*/

// in bytes
static const uint32_t g_iHeaderOffset = 0;
static const uint32_t g_iSizeOffset = 4;
static const uint32_t g_iDataOffset = 8;

// private
void* ck_get_raw(void* in_pv)
{
   return (uint8_t*)in_pv-g_iDataOffset;
}

void* ck_get_data(void* in_pvck)
{
   return (uint8_t*)in_pvck+g_iDataOffset;
}

void* ck_alloc(const uint32_t in_i)
{
	// data + ckid + cksize
	void* pv = nn_allocmsg(in_i+g_iDataOffset, 0);

   if (pv!=NULL) {
      // the following calls expect to deal with a data ptr
      pv = ck_get_data(pv);

	   // store the size
      ck_set_size(pv, in_i);

      // reset the header
      ck_set_header(pv, 0);
   }
   // return a pointer to the data
	return pv;
}

int ck_free(void* in_pv)
{
   if (in_pv!=NULL)
	   return nn_freemsg(ck_get_raw(in_pv));
   return 0;
}

void* ck_realloc(void* in_pv, const uint32_t in_i)
{
   void* pv;

   // so nanomsg doesn't do realloc with a null pointer, patch that here by calling an alloc in that situation
   if (in_pv==NULL)
      return ck_alloc(in_i);
	pv = nn_reallocmsg(ck_get_raw(in_pv), in_i+g_iDataOffset);
   if (pv!=NULL) {
      // point to the data
      pv = ck_get_data(pv);

      // update the size
      ck_set_size(pv, in_i);
   }
   return pv;
}

void ck_set_header(void* in_pvck, const uint32_t in_ui32Header)
{
   // in_pvck is pointing to data, offset it
	*((uint32_t*)ck_get_raw(in_pvck)+(g_iHeaderOffset>>2)) = in_ui32Header;
}

void ck_copy_data(void* io_pvck, const void* in_pvsrc, const uint32_t in_ui32)
{
   // io_pvck is already pointing to data
	memcpy((uint8_t*)io_pvck, in_pvsrc, in_ui32);

   // set the size
   ck_set_size(io_pvck, in_ui32);
}

const void* ck_get_header(void* in_pvck)
{
	return (uint8_t*)ck_get_raw(in_pvck)+g_iHeaderOffset;
}

uint32_t ck_get_size(void* in_pvck)
{
	return *((uint8_t*)ck_get_raw(in_pvck)+g_iSizeOffset);
}

void ck_set_size(void* in_pvck, const uint32_t in_ui32)
{
   *((uint32_t*)ck_get_raw(in_pvck)+(g_iSizeOffset>>2)) = in_ui32;
}