// ==============================================================================
//	Last modification of initial version
//					Sep.8 5:51pm
// ==============================================================================
#include "decwin.h"
/*//reduced dct coefficient 
0.500603  0.505471  0.515447  0.531043  0.553104  0.582935  0.622504  0.674808	//used in cos16	
0.744536  0.839350  0.972568  1.169440	1.484165  2.057781  3.407609  10.190008	

0.502419  0.522499  0.566944  0.646822  0.788155  1.060678  1.722447  5.101149	//used in cos8
0.509796  0.601345  0.899976  2.562916	0.541196  1.306563  0.707107		*/

static void dct64(float *out0,float *out1,float *b1,float *b2,float *samples)
{
	float tmp;
	if(1)	//decrease the diversion b1->b2->b1->b2...->out0,1
    {	//16+16,b1[ 0-15] =  s[ 0-15] + s[31-16]
	//	b1[16-31] = (s[15-0 ] - s[16-31])*cos16[0-15]
  	b1[0x00] = samples[0x00] + samples[0x1F];
  	b1[0x01] = samples[0x01] + samples[0x1E];
  	b1[0x1F] = (samples[0x00] - samples[0x1F]) * 0.500603; 
  	b1[0x1E] = (samples[0x01] - samples[0x1E]) * 0.505471; 
  	b1[0x02] = samples[0x02] + samples[0x1D];
  	b1[0x03] = samples[0x03] + samples[0x1C];
  	b1[0x1D] = (samples[0x02] - samples[0x1D]) * 0.515447; 
  	b1[0x1C] = (samples[0x03] - samples[0x1C]) * 0.531043;
  	b1[0x04] = samples[0x04] + samples[0x1B];
  	b1[0x05] = samples[0x05] + samples[0x1A];
  	b1[0x1B] = (samples[0x04] - samples[0x1B]) * 0.553104;
  	b1[0x1A] = (samples[0x05] - samples[0x1A]) * 0.582935;
  	b1[0x06] = samples[0x06] + samples[0x19];
  	b1[0x07] = samples[0x07] + samples[0x18];
  	b1[0x19] = (samples[0x06] - samples[0x19]) * 0.622504;
  	b1[0x18] = (samples[0x07] - samples[0x18]) * 0.674808;
  	b1[0x08] = samples[0x08] + samples[0x17];
  	b1[0x09] = samples[0x09] + samples[0x16];
  	b1[0x17] = (samples[0x08] - samples[0x17]) * 0.744536;
  	b1[0x16] = (samples[0x09] - samples[0x16]) * 0.839350;
  	b1[0x0A] = samples[0x0A] + samples[0x15];
  	b1[0x0B] = samples[0x0B] + samples[0x14];
  	b1[0x15] = (samples[0x0A] - samples[0x15]) * 0.972568;
  	b1[0x14] = (samples[0x0B] - samples[0x14]) * 1.169440;
  	b1[0x0C] = samples[0x0C] + samples[0x13];
  	b1[0x0D] = samples[0x0D] + samples[0x12];
  	b1[0x13] = (samples[0x0C] - samples[0x13]) * 1.484165;
  	b1[0x12] = (samples[0x0D] - samples[0x12]) * 2.057781;
  	b1[0x0E] = samples[0x0E] + samples[0x11];
  	b1[0x0F] = samples[0x0F] + samples[0x10];
  	b1[0x11] = (samples[0x0E] - samples[0x11]) * 3.407609;
  	b1[0x10] = (samples[0x0F] - samples[0x10]) * 10.190008; 
   }

   {// 8+8+8+8	b2[ 0-7 ] =  b1[ 0-7 ] + b1[15-8 ]
	//	b2[ 8-15] = (b1[ 7-0 ] - b1[ 8-15])*cos8[0-7]
	//	b2[16-23] =  b1[16-23] + b1[31-24]
	//	b2[24-31] = (b1[24-31] - b1[23-16])*cos8[0-7]
  	b2[0x00] = b1[0x00] + b1[0x0F]; 
 	b2[0x01] = b1[0x01] + b1[0x0E]; 
  	b2[0x0F] = (b1[0x00] - b1[0x0F]) * 0.502419;
  	b2[0x0E] = (b1[0x01] - b1[0x0E]) * 0.522499;
  	b2[0x02] = b1[0x02] + b1[0x0D]; 
  	b2[0x03] = b1[0x03] + b1[0x0C]; 
  	b2[0x0D] = (b1[0x02] - b1[0x0D]) * 0.566944;
  	b2[0x0C] = (b1[0x03] - b1[0x0C]) * 0.646822;
  	b2[0x04] = b1[0x04] + b1[0x0B]; 
  	b2[0x05] = b1[0x05] + b1[0x0A]; 
  	b2[0x0B] = (b1[0x04] - b1[0x0B]) * 0.788155;
  	b2[0x0A] = (b1[0x05] - b1[0x0A]) * 1.060678;
  	b2[0x06] = b1[0x06] + b1[0x09]; 
  	b2[0x07] = b1[0x07] + b1[0x08]; 
  	b2[0x09] = (b1[0x06] - b1[0x09]) * 1.722447;
  	b2[0x08] = (b1[0x07] - b1[0x08]) * 5.101149;

  	b2[0x10] = b1[0x10] + b1[0x1F];
  	b2[0x11] = b1[0x11] + b1[0x1E];
  	b2[0x1F] = (b1[0x1F] - b1[0x10]) * 0.502419;
  	b2[0x1E] = (b1[0x1E] - b1[0x11]) * 0.522499;
  	b2[0x12] = b1[0x12] + b1[0x1D];
  	b2[0x13] = b1[0x13] + b1[0x1C];
  	b2[0x1D] = (b1[0x1D] - b1[0x12]) * 0.566944;
  	b2[0x1C] = (b1[0x1C] - b1[0x13]) * 0.646822;
  	b2[0x14] = b1[0x14] + b1[0x1B];
  	b2[0x15] = b1[0x15] + b1[0x1A];
  	b2[0x1B] = (b1[0x1B] - b1[0x14]) * 0.788155;
  	b2[0x1A] = (b1[0x1A] - b1[0x15]) * 1.060678;
  	b2[0x16] = b1[0x16] + b1[0x19];
  	b2[0x17] = b1[0x17] + b1[0x18];
  	b2[0x19] = (b1[0x19] - b1[0x16]) * 1.722447;
  	b2[0x18] = (b1[0x18] - b1[0x17]) * 5.101149;
   }
/////////////////////////////////////////////////////////////////////////////////
   {//4+4,4+4, 4+4, 4+4 
  	b1[0x00] = b2[0x00] + b2[0x07];
  	b1[0x07] = (b2[0x00] - b2[0x07]) * 0.509796;
  	b1[0x01] = b2[0x01] + b2[0x06];
  	b1[0x06] = (b2[0x01] - b2[0x06]) * 0.601345;
  	b1[0x02] = b2[0x02] + b2[0x05];
  	b1[0x05] = (b2[0x02] - b2[0x05]) * 0.899976;
  	b1[0x03] = b2[0x03] + b2[0x04];
  	b1[0x04] = (b2[0x03] - b2[0x04]) * 2.562916;

  	b1[0x08] = b2[0x08] + b2[0x0F];
  	b1[0x0F] = (b2[0x0F] - b2[0x08]) * 0.509796;
  	b1[0x09] = b2[0x09] + b2[0x0E];
  	b1[0x0E] = (b2[0x0E] - b2[0x09]) * 0.601345;
  	b1[0x0A] = b2[0x0A] + b2[0x0D];
  	b1[0x0D] = (b2[0x0D] - b2[0x0A]) * 0.899976;
  	b1[0x0B] = b2[0x0B] + b2[0x0C];
  	b1[0x0C] = (b2[0x0C] - b2[0x0B]) * 2.562916;

  	b1[0x10] = b2[0x10] + b2[0x17];
  	b1[0x17] = (b2[0x10] - b2[0x17]) * 0.509796;
  	b1[0x11] = b2[0x11] + b2[0x16];
  	b1[0x16] = (b2[0x11] - b2[0x16]) * 0.601345;
  	b1[0x12] = b2[0x12] + b2[0x15];
  	b1[0x15] = (b2[0x12] - b2[0x15]) * 0.899976;
  	b1[0x13] = b2[0x13] + b2[0x14];
  	b1[0x14] = (b2[0x13] - b2[0x14]) * 2.562916;

  	b1[0x18] = b2[0x18] + b2[0x1F];
  	b1[0x1F] = (b2[0x1F] - b2[0x18]) * 0.509796;
  	b1[0x19] = b2[0x19] + b2[0x1E];
  	b1[0x1E] = (b2[0x1E] - b2[0x19]) * 0.601345;
  	b1[0x1A] = b2[0x1A] + b2[0x1D];
  	b1[0x1D] = (b2[0x1D] - b2[0x1A]) * 0.899976;
  	b1[0x1B] = b2[0x1B] + b2[0x1C];
  	b1[0x1C] = (b2[0x1C] - b2[0x1B]) * 2.562916;
   }

   {//2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
  	b2[0x00] = b1[0x00] + b1[0x03];
  	b2[0x03] = (b1[0x00] - b1[0x03]) * 0.541196;
  	b2[0x01] = b1[0x01] + b1[0x02];
  	b2[0x02] = (b1[0x01] - b1[0x02]) * 1.306563;
  	b2[0x04] = b1[0x04] + b1[0x07];
  	b2[0x07] = (b1[0x07] - b1[0x04]) * 0.541196;
  	b2[0x05] = b1[0x05] + b1[0x06];
  	b2[0x06] = (b1[0x06] - b1[0x05]) * 1.306563;

  	b2[0x08] = b1[0x08] + b1[0x0B];
  	b2[0x0B] = (b1[0x08] - b1[0x0B]) * 0.541196;
  	b2[0x09] = b1[0x09] + b1[0x0A];
  	b2[0x0A] = (b1[0x09] - b1[0x0A]) * 1.306563;  
  	b2[0x0C] = b1[0x0C] + b1[0x0F];
  	b2[0x0F] = (b1[0x0F] - b1[0x0C]) * 0.541196;
  	b2[0x0D] = b1[0x0D] + b1[0x0E];
  	b2[0x0E] = (b1[0x0E] - b1[0x0D]) * 1.306563;

  	b2[0x10] = b1[0x10] + b1[0x13];
  	b2[0x13] = (b1[0x10] - b1[0x13]) * 0.541196;
  	b2[0x11] = b1[0x11] + b1[0x12];
  	b2[0x12] = (b1[0x11] - b1[0x12]) * 1.306563;
  	b2[0x14] = b1[0x14] + b1[0x17];
  	b2[0x17] = (b1[0x17] - b1[0x14]) * 0.541196;
  	b2[0x15] = b1[0x15] + b1[0x16];
  	b2[0x16] = (b1[0x16] - b1[0x15]) * 1.306563;

  	b2[0x18] = b1[0x18] + b1[0x1B];
  	b2[0x1B] = (b1[0x18] - b1[0x1B]) * 0.541196;
  	b2[0x19] = b1[0x19] + b1[0x1A];
  	b2[0x1A] = (b1[0x19] - b1[0x1A]) * 1.306563;
  	b2[0x1C] = b1[0x1C] + b1[0x1F];
  	b2[0x1F] = (b1[0x1F] - b1[0x1C]) * 0.541196;
  	b2[0x1D] = b1[0x1D] + b1[0x1E];
  	b2[0x1E] = (b1[0x1E] - b1[0x1D]) * 1.306563;
   }

   {
  	b1[0x00] = b2[0x00] + b2[0x01];
  	b1[0x01] = (b2[0x00] - b2[0x01]) * 0.707107;
  	b1[0x02] = b2[0x02] + b2[0x03];
  	b1[0x03] = (b2[0x03] - b2[0x02]) * 0.707107;
  	b1[0x02] += b1[0x03];

  	b1[0x04] = b2[0x04] + b2[0x05];
  	b1[0x05] = (b2[0x04] - b2[0x05]) * 0.707107;
  	b1[0x06] = b2[0x06] + b2[0x07];
  	b1[0x07] = (b2[0x07] - b2[0x06]) * 0.707107;
  	b1[0x06] += b1[0x07];
  	b1[0x04] += b1[0x06];
  	b1[0x06] += b1[0x05];
  	b1[0x05] += b1[0x07];

  	b1[0x08] = b2[0x08] + b2[0x09];
  	b1[0x09] = (b2[0x08] - b2[0x09]) * 0.707107;
  	b1[0x0A] = b2[0x0A] + b2[0x0B];
  	b1[0x0B] = (b2[0x0B] - b2[0x0A]) * 0.707107;
  	b1[0x0A] += b1[0x0B];

  	b1[0x0C] = b2[0x0C] + b2[0x0D];
  	b1[0x0D] = (b2[0x0C] - b2[0x0D]) * 0.707107;
  	b1[0x0E] = b2[0x0E] + b2[0x0F];
  	b1[0x0F] = (b2[0x0F] - b2[0x0E]) * 0.707107;
  	b1[0x0E] += b1[0x0F];
  	b1[0x0C] += b1[0x0E];
  	b1[0x0E] += b1[0x0D];
  	b1[0x0D] += b1[0x0F];

  	b1[0x10] = b2[0x10] + b2[0x11];
  	b1[0x11] = (b2[0x10] - b2[0x11]) * 0.707107;
  	b1[0x12] = b2[0x12] + b2[0x13];
  	b1[0x13] = (b2[0x13] - b2[0x12]) * 0.707107;
  	b1[0x12] += b1[0x13];

  	b1[0x14] = b2[0x14] + b2[0x15];
  	b1[0x15] = (b2[0x14] - b2[0x15]) * 0.707107;
  	b1[0x16] = b2[0x16] + b2[0x17];
  	b1[0x17] = (b2[0x17] - b2[0x16]) * 0.707107;
  	b1[0x16] += b1[0x17];
  	b1[0x14] += b1[0x16];
  	b1[0x16] += b1[0x15];
  	b1[0x15] += b1[0x17];

  	b1[0x18] = b2[0x18] + b2[0x19];
  	b1[0x19] = (b2[0x18] - b2[0x19]) * 0.707107;
  	b1[0x1A] = b2[0x1A] + b2[0x1B];
  	b1[0x1B] = (b2[0x1B] - b2[0x1A]) * 0.707107;
  	b1[0x1A] += b1[0x1B];

  	b1[0x1C] = b2[0x1C] + b2[0x1D];
  	b1[0x1D] = (b2[0x1C] - b2[0x1D]) * 0.707107;
  	b1[0x1E] = b2[0x1E] + b2[0x1F];
  	b1[0x1F] = (b2[0x1F] - b2[0x1E]) * 0.707107;
  	b1[0x1E] += b1[0x1F];
  	b1[0x1C] += b1[0x1E];
  	b1[0x1E] += b1[0x1D];
  	b1[0x1D] += b1[0x1F];
   }
	//even 	out0[0,2,4,6,8,10,12,14,16]*16
	//	out1[0,2,4,6,8,10,12,14,16]*16
 	out0[256] = b1[0x00];
 	out0[192] = b1[0x04];
 	out0[128] = b1[0x02];
 	out0[64 ] = b1[0x06];
 	out0[0  ] = b1[0x01];
 	out1[0  ] = b1[0x01];
 	out1[64 ] = b1[0x05];
 	out1[128] = b1[0x03];
 	out1[192] = b1[0x07];
	out1[256] = b1[0x00];

 	out0[224] = b1[0x08] + b1[0x0C];
 	out0[160] = b1[0x0C] + b1[0x0a];
 	out0[96 ] = b1[0x0A] + b1[0x0E];
 	out0[32 ] = b1[0x0E] + b1[0x09];
 	out1[32 ] = b1[0x09] + b1[0x0D];
 	out1[96 ] = b1[0x0D] + b1[0x0B];
 	out1[160] = b1[0x0B] + b1[0x0F];
 	out1[224] = b1[0x0F];

	//odd
 	tmp = b1[0x18] + b1[0x1C];
 	out0[240] = tmp + b1[0x10];
 	out0[208] = tmp + b1[0x14];
 	tmp =  b1[0x1C] + b1[0x1A];
 	out0[176] = tmp + b1[0x14];
 	out0[144] = tmp + b1[0x12];
 	tmp =  b1[0x1A] + b1[0x1E];
 	out0[112] = tmp + b1[0x12];
 	out0[80 ] = tmp + b1[0x16];
 	tmp =  b1[0x1E] + b1[0x19];
 	out0[48 ] = tmp + b1[0x16];
 	out0[16 ] = tmp + b1[0x11];
 	tmp =  b1[0x19] + b1[0x1D];
 	out1[16 ] = tmp + b1[0x11];
 	out1[48 ] = tmp + b1[0x15]; 
 	tmp =  b1[0x1D] + b1[0x1B];
 	out1[80 ] = tmp + b1[0x15];
 	out1[112] = tmp + b1[0x13];
 	tmp =  b1[0x1B] + b1[0x1F];
 	out1[144] = tmp + b1[0x13];
 	out1[176] = tmp + b1[0x17];
 	out1[208] = b1[0x17] + b1[0x1F];
 	out1[240] = b1[0x1F];
}
// ------------------------------------------------------------------------------
// synthesize using polyphase MDCT 
// ------------------------------------------------------------------------------ 
void synthesize(float * xrptr, int channel, unsigned char *out, int *pnt)
{
	static float buffs[2][2][1024];
	static int bo = 1;
	short *samples = (short *) (out + *pnt);
	float tempbuf[64];

	float *b0, (*buf)[1024];
	int bo1;

	register int j;
	float *window;
	float sum;

	//select buffer memory for xr
	if (!channel)
	{
		bo--;
		bo &= 0xf;
		buf = buffs[0];
	}
	else
	{
		samples++;
		buf = buffs[1];
	}
	//input 32*1 vector
	if (bo & 0x1)
	{
		b0 = buf[0];
		bo1 = bo;
		dct64(buf[1]+((bo+1)&0xf), buf[0]+bo, tempbuf, tempbuf+32, xrptr);
	}
	else
	{
		b0 = buf[1];
		bo1 = bo + 1;
		dct64(buf[0]+bo,buf[1]+bo+1,tempbuf, tempbuf+32, xrptr);
	}
	// 512 vector multiply the coefficient and then compute the sample, ouput 32sample
	window = decwin + 16 - bo1;
	for (j = 16; j; j--, window += 0x10, samples += 2)
	{
		sum  = *window++ * *b0++;
		sum -= *window++ * *b0++;
		sum += *window++ * *b0++;
		sum -= *window++ * *b0++;
		sum += *window++ * *b0++;
		sum -= *window++ * *b0++;
		sum += *window++ * *b0++;
		sum -= *window++ * *b0++;
		sum += *window++ * *b0++;
		sum -= *window++ * *b0++;
		sum += *window++ * *b0++;
		sum -= *window++ * *b0++;
		sum += *window++ * *b0++;
		sum -= *window++ * *b0++;
		sum += *window++ * *b0++;
		sum -= *window++ * *b0++;
		if( (sum) > 32767.0) 
			*(samples) = 0x7fff; 
		else if( (sum) < -32768.0) 
 			*(samples) = -0x8000; 
		else  
			*(samples) = sum; 
	}
#if 1
	sum =  window[0x0] * b0[0x0];
	sum += window[0x2] * b0[0x2];
	sum += window[0x4] * b0[0x4];
	sum += window[0x6] * b0[0x6];
	sum += window[0x8] * b0[0x8];
	sum += window[0xA] * b0[0xA];
	sum += window[0xC] * b0[0xC];
	sum += window[0xE] * b0[0xE];
	if( (sum) > 32767.0) 
		*(samples) = 0x7fff; 
	else if( (sum) < -32768.0) 
 		*(samples) = -0x8000; 
	else  
		*(samples) = sum; 
	b0 -= 0x10, window -= 0x20, samples += 2;
	
#endif
#if 1
	window += bo1 << 1;
	for (j = 15; j; j--, b0 -= 0x20, window -= 0x10, samples += 2)
	{
		sum  = -*(--window) * *b0++;
		sum -= *(--window) * *b0++;
		sum -= *(--window) * *b0++;
		sum -= *(--window) * *b0++;
		sum -= *(--window) * *b0++;
		sum -= *(--window) * *b0++;
		sum -= *(--window) * *b0++;
		sum -= *(--window) * *b0++;
		sum -= *(--window) * *b0++;
		sum -= *(--window) * *b0++;
		sum -= *(--window) * *b0++;
		sum -= *(--window) * *b0++;
		sum -= *(--window) * *b0++;
		sum -= *(--window) * *b0++;
		sum -= *(--window) * *b0++;
		sum -= *(--window) * *b0++;
		if( (sum) > 32767.0) 
			*(samples) = 0x7fff; 
		else if( (sum) < -32768.0) 
 			*(samples) = -0x8000; 
		else  
			*(samples) = sum; 
	}
#endif
	*pnt += 128;
}
