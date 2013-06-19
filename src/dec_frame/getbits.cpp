// ==============================================================================
//	Last modification of the initial version.
//					Set.8 5:47pm
// ==============================================================================
#include "getbits.h"
#include "frame.h"

extern struct bit_info bi;
// ------------------------------------------------------------------------------
// the max byte may be taken up is 3, 
// 9bit, ptr[0]=8bit, ptr[1]=1bit, ptr[2]=0, index=0 supposed 
// ------------------------------------------------------------------------------
unsigned int getbits(int number)
{
	unsigned long ret; //0,0,0,0
	if(!number)
		return 0;
	ret = (bi.ptr[0])<<8;		//0, 0, A, 0
	ret = (ret|bi.ptr[1])<<8;	//0, A, B, 0
	ret = (ret|bi.ptr[2]);		//0, A, B, C

	ret = (ret<<bi.index)&0xffffff; //
	bi.index += number;		//80,aaaa,aaaa,bbbb,bbbb,cccc,cccc
	ret = ret>>(24-number);		//80,0000,0000,0000,000a,aaaa,aaab      
	
	bi.ptr += (bi.index>>3);	//set to the next index, here is 1
	bi.index &= 0x7;		
	
	return ret;	
}
// ------------------------------------------------------------------------------
// get 1 bit from bit stream, return its value
// ------------------------------------------------------------------------------
unsigned int get1bit(void)
{
	unsigned char ret;		//0000,0000
	ret = *bi.ptr << bi.index;	//buf[n] << index

	bi.index++;			//setting the index
	bi.ptr += (bi.index>>3);
	bi.index &= 7;
	return (ret>>7);
}
// ------------------------------------------------------------------------------
// get 1 bytes from bit stream, return the byte's value
// ------------------------------------------------------------------------------
int getbyte(void)
{
  return (*bi.ptr++);
}
// ------------------------------------------------------------------------------
// get the offset in the bit stream
// ------------------------------------------------------------------------------
int getbitoffset(void) 
{
  return (-bi.index)&0x7;
}
// ------------------------------------------------------------------------------
// back n bits 
// ------------------------------------------------------------------------------
void backbits(int number_of_bits)
{
  bi.index -= number_of_bits;
  bi.ptr += (bi.index>>3);
  bi.index &= 0x7;
}
