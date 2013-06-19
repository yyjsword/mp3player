// ==============================================================================
//	last modification of the initial verion.
//				--Sep.8 5:55pm
// ==============================================================================
#include <stdio.h>
#include <math.h>
#include "huffman.h"
#include "getbits.h"
#include "frame.h"
#include "../mp.h"



extern unsigned char *g_pcm_sample;
extern int g_pcm_point;
extern void synthesize(float*,int,unsigned char*,int*);
int pcm_point=0;


static float ispow[8207]={0,};
static float ca[8]={0,},cs[8]={0,};
static float win[4][36]={{0,}};
static float win1[4][36]={{0,}};
static float gainpow2[256+118+4]={0,};//378


struct bandInfoStruct bandInfo[6] = { 
/* MPEG 1.0 */
 { {0,4,8,12,16,20,24,30,36,44,52,62,74, 90,110,134,162,196,238,288,342,418,576},
   {4,4,4,4,4,4,6,6,8, 8,10,12,16,20,24,28,34,42,50,54, 76,158},
   {0,4*3,8*3,12*3,16*3,22*3,30*3,40*3,52*3,66*3, 84*3,106*3,136*3,192*3},
   {4,4,4,4,6,8,10,12,14,18,22,30,56} } ,

 { {0,4,8,12,16,20,24,30,36,42,50,60,72, 88,106,128,156,190,230,276,330,384,576},
   {4,4,4,4,4,4,6,6,6, 8,10,12,16,18,22,28,34,40,46,54, 54,192},
   {0,4*3,8*3,12*3,16*3,22*3,28*3,38*3,50*3,64*3, 80*3,100*3,126*3,192*3},
   {4,4,4,4,6,6,10,12,14,16,20,26,66} } ,

 { {0,4,8,12,16,20,24,30,36,44,54,66,82,102,126,156,194,240,296,364,448,550,576} ,
   {4,4,4,4,4,4,6,6,8,10,12,16,20,24,30,38,46,56,68,84,102, 26} ,
   {0,4*3,8*3,12*3,16*3,22*3,30*3,42*3,58*3,78*3,104*3,138*3,180*3,192*3} ,
   {4,4,4,4,6,8,12,16,20,26,34,42,12} } ,
/* MPEG 2.0 */
 { {0,6,12,18,24,30,36,44,54,66,80,96,116,140,168,200,238,284,336,396,464,522,576},
   {6,6,6,6,6,6,8,10,12,14,16,20,24,28,32,38,46,52,60,68,58,54 } ,
   {0,4*3,8*3,12*3,18*3,24*3,32*3,42*3,56*3,74*3,100*3,132*3,174*3,192*3} ,
   {4,4,4,6,6,8,10,14,18,26,32,42,18 } } ,

 { {0,6,12,18,24,30,36,44,54,66,80,96,114,136,162,194,232,278,330,394,464,540,576},
   {6,6,6,6,6,6,8,10,12,14,16,18,22,26,32,38,46,52,64,70,76,36 } ,
   {0,4*3,8*3,12*3,18*3,26*3,36*3,48*3,62*3,80*3,104*3,136*3,180*3,192*3} ,
   {4,4,4,6,8,10,12,14,18,24,32,44,12 } } ,

 { {0,6,12,18,24,30,36,44,54,66,80,96,116,140,168,200,238,284,336,396,464,522,576},
   {6,6,6,6,6,6,8,10,12,14,16,20,24,28,32,38,46,52,60,68,58,54 },
   {0,4*3,8*3,12*3,18*3,26*3,36*3,48*3,62*3,80*3,104*3,134*3,174*3,192*3},
   {4,4,4,6,8,10,12,14,18,24,30,40,18 } } ,
};
static int mapbuf0[9][152]={{0,}};
static int mapbuf1[9][156]={{0,}};
static int mapbuf2[9][44]={{0,}};
static int *map[9][3]={{0,}};
static int *mapend[9][3]={{0,}};

#define REFRESH_MASK \
  while(num < 24) { \
    mask |= ((unsigned long)getbyte())<<(24-num); \
    num += 8; \
    hcodes -= 8; }
// ------------------------------------------------------------------------------
//	init the variable value used in decode & quantize and math transformat 
//	some of them must be placed out of the main loop.
// ------------------------------------------------------------------------------
void yyj_init_layer3(void)
{
  	int i,j;
	double Ci[8]={-0.6,-0.535,-0.33,-0.185,-0.095,-0.041,-0.0142,-0.0037};
	double sq;
  	for(i=-256;i<118+4;i++)
      		gainpow2[i+256] = pow(2.0,-0.25*(i+210));	// << != */
  	for(i=0;i<2048;i++)	//quantize factor, compute here is for quality
    		ispow[i] = pow(i,4.0/3.0); 
	
  	for (i=0;i<8;i++) 
	{
    		sq=sqrt(1.0+Ci[i]*Ci[i]);
    		cs[i] = 1.0/sq;
    		ca[i] = Ci[i]/sq;
  	}
  	// compute window used in IMDCT
	// win[0] 	normal	b_t=0	36	(0-35)
	// win[1]	start  	b_t=1	36	(0-17) (18-23) (24-29) (30-35)
	// win[2]	short	b_t=2	11	(0-11)
	// win[3]	stop	b_t=3	36	(0-5) (6-11) (12-17) (18-35)
  	for(i=0;i<18;i++) 	//win[0][0-35] 	win[1][0-17] 	win[3][18-35] 
	{
    		win[0][i]=win[1][i]= 0.5*sin(PI/72*(2*i+1))/cos(PI*(2*i+19)/72.0);
    		win[0][i+18]=win[3][i+18]= 0.5*sin(PI/72*(2*i+37))/cos(PI*(2*i+55)/72);
  	}
  	for(i=0;i<6;i++) 	// 		win[1][18-35] 	win[3][0-17]
	{
    		win[1][i+18] = 0.5/cos(PI*(2*(i+18)+19)/72.0);
    		win[3][i+12] = 0.5/cos(PI*(2*(i+12)+19)/72.0);
    		win[1][i+24] = 0.5*sin(PI/24.0*(2*i+13))/cos(PI*(2*(i+24)+19)/72.0);
    		win[1][i+30] = win[3][i] = 0.0;
    		win[3][i+6 ] = 0.5*sin(PI/24.0*(2*i+1) )/cos(PI*(2*(i+6)+19)/72.0);
  	}
	for(i=0;i<12;i++)	// win[2][0-11]
    		win[2][i] = 0.5*sin(PI/24.0*(2*i+1))/cos(PI*(2*i+7)/24.0);
  	
  	for(j=0;j<4;j++) 
	{
    		static int len[4] = { 36,36,12,36 };
    		for(i=0;i<len[j];i+=2)
      			win1[j][i] = + win[j][i];
    		for(i=1;i<len[j];i+=2)
      			win1[j][i] = - win[j][i];
 	}

	for(j=0;j<6;j++) 
	{
   		struct bandInfoStruct *bi = &bandInfo[j];
  		int *mp;
   		int cb,lwin;
   		int *bdf;

  		mp = map[j][0] = mapbuf0[j];
   		bdf = bi->longDiff;
   		for(i=0,cb = 0; cb < 8 ; cb++,i+=*bdf++) 
		{
     			*mp++ = (*bdf) >> 1;
     			*mp++ = i;
     			*mp++ = 3;
     			*mp++ = cb;
   		}
   		bdf = bi->shortDiff+3;
   		for(cb=3;cb<13;cb++) 
		{
     			int l = (*bdf++) >> 1;
     			for(lwin=0;lwin<3;lwin++) 
			{
       				*mp++ = l;
       				*mp++ = i + lwin;
       				*mp++ = lwin;
       				*mp++ = cb;
     			}
     			i += 6*l;
   		}
   		mapend[j][0] = mp;

   		mp = map[j][1] = mapbuf1[j];
   		bdf = bi->shortDiff+0;
   		for(i=0,cb=0;cb<13;cb++) 
		{
     			int l = (*bdf++) >> 1;
     			for(lwin=0;lwin<3;lwin++) 
			{
       				*mp++ = l;
       				*mp++ = i + lwin;
       				*mp++ = lwin;
       				*mp++ = cb;
     			}
     			i += 6*l;
   		}
   		mapend[j][1] = mp;

   		mp = map[j][2] = mapbuf2[j];
   		bdf = bi->longDiff;
   		for(cb = 0; cb < 22 ; cb++) 
		{
     			*mp++ = (*bdf++) >> 1;
     			*mp++ = cb;
   		}
   		mapend[j][2] = mp;
  	}
}
// ------------------------------------------------------------------------------
//	get side info 
//		return value: 
//			0 - success
//			1 - failure 
// ------------------------------------------------------------------------------
static int get_side_info(struct sideinfo *si,int stereo,long sfreq,int lsf, struct frame *fr)
{
   	int ch, gr;
   	int i;
   	si->main_data_begin = getbits(9);
   	if (stereo == 1)
     		si->private_bits = getbits(5);
   	else 
     		si->private_bits = getbits(3);
     	for (ch=0; ch<stereo; ch++) 
	{
         	si->ch[ch].gr[0].scfsi = -1;
         	si->ch[ch].gr[1].scfsi = getbits(4);
     	}
   	for (gr=0; gr<2; gr++)
     		for (ch=0; ch<stereo; ch++) 
		{
       			struct gr_info *gi = &(si->ch[ch].gr[gr]);	
       			gi->part2_3_length = getbits(12);
       			gi->big_values = getbits(9);
       			if(gi->big_values > 288) 
          			gi->big_values = 288;
       			gi->pow2gain = gainpow2 + 256 - getbits(8);
       			gi->scalefac_compress = getbits(4);
       			if(get1bit())	// blocksplit_flag, useless
			{ 
         			gi->block_type = getbits(2);
         			gi->mixed_block_flag = get1bit();
				for(i=0;i<2;i++)
         				gi->table_select[i]  = getbits(5);
    			       	gi->table_select[2] = 0;
         			for(i=0;i<3;i++)
				{
          				gi->subblock_gain[i] = getbits(3);
					gi->full_gain[i] = gi->pow2gain+(gi->subblock_gain[i]<<3);
				}
         			if(gi->block_type == 0) 
           				return 1;
      				// setting the region start address. 36, 576
         			if(!lsf || gi->block_type == 2)
           				gi->region1 = 36>>1;
         			else {
           				if(sfreq == 8)
             					gi->region1 = 108>>1;
           				else
             					gi->region1 = 54>>1;
         			}
         			gi->region2 = 576>>1;
       			}
       			else 
			{
         			int r0c,r1c;
         			for (i=0; i<3; i++)
           				gi->table_select[i] = getbits(5);
         			r0c = getbits(4);
         			r1c = getbits(3);
         			gi->region1 = bandInfo[sfreq].longIdx[r0c+1] >> 1 ;
         			gi->region2 = bandInfo[sfreq].longIdx[r0c+1+r1c+1] >> 1;
         			gi->block_type = 0;
         			gi->mixed_block_flag = 0;
       			}
			// Modified Sep13
			if(!lsf)
         			gi->preflag = get1bit();
       			gi->scalefac_scale = get1bit();
       			gi->count1table_select = get1bit();
     		}
   	return 0;
}
// ------------------------------------------------------------------------------
//	get scale factors 
//		return value: 
//			length of sacle factor	
// ------------------------------------------------------------------------------
static int get_scale_factors(int *sf,struct gr_info *gi)
{
//defined in ISO 11172-3,defined the length of scale factor
unsigned char slen[2][16] = {	
		{0,0,0,0, 3,1,1,1, 2,2,2,3, 3,3,4,4},
		{0,1,2,3, 0,1,2,3, 1,2,3,1, 2,3,2,3} };
   	int ret = 0;
   	int scale0 = slen[0][gi->scalefac_compress];
   	int scale1 = slen[1][gi->scalefac_compress];

    	if (gi->block_type == 2) 
	{
      		int i=18;
      		ret = (scale0 + scale1) * 18;

      		if (gi->mixed_block_flag) 
		{
        	// b_t=2,s_p=1,slen1 0-7 for long window scalefactor, 4-11 for short slen2 6-11
			ret = 17*scale0 + 18*scale1;
			for(i=0;i<8;i++)
				*sf++ = getbits(scale0);
			i = 9; 
      		}
     		for(; i; i--)
			*sf++ = getbits(scale0);
		for(i=0; i<18; i++)
			*sf++ = getbits(scale1);
		*sf++ = 0;
		*sf++ = 0;
		*sf++ = 0;
    		}
    		else 
		{
   			int i;
      			if(gi->scfsi < 0)				//scfsi<0 => granule==0
			{ 			
        			ret = (scale0 + scale1) * 10 + scale0; 	//b_t = 0,1,3; s_p =0
				for(i=0;i<11;i++)		
           				*sf++ = getbits(scale0);
         			for(i=0;i<10;i++)
           				*sf++ = getbits(scale1);
               			*sf++ = 0;
      			}
      			else 
			{
        			if(!(gi->scfsi & 0x8)) 
				{
          				for (i=0;i<6;i++)
            					*sf++ = getbits(scale0);
          				ret += scale0 * 6;
        			}
        			else 
          				sf += 6;        

        			if(!(gi->scfsi & 0x4)) 
				{
          				for (i=0;i<5;i++)
            					*sf++ = getbits(scale0);
          				ret += scale0 * 5;
        			}
        			else
          				sf += 5;
        
	        		if(!(gi->scfsi & 0x2)) 
				{
          				for(i=0;i<5;i++)
            					*sf++ = getbits(scale1);
          				ret += scale1 * 5;
        			}
        			else 
          				sf += 5; 

        			if(!(gi->scfsi & 0x1)) 
				{
          				for (i=0;i<5;i++)
            					*sf++ = getbits(scale1);
          				ret += scale1 * 5;
        			}
        			else 
           				sf += 5;
        	
        			*sf++ = 0;  
      			}
    		}
    	return ret;
}
static short t0[] ={-29,-21,-13,-7,-3,-1,11,15,-1,13,14,-3,-1,7,5,9,-3,-1,6,3,-1,10,12,-3,-1,2,1,-1,4,8,0};
static short t1[] ={-15,-7,-3,-1,15,14,-1,13,12,-3,-1,11,10,-1,9,8,-7,-3,-1,7,6,-1,5,4,-3,-1,3,2,-1,1,0};
static int pretab1[21] = {0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,2,2,3,3,3,2};
static int pretab2[21] = {0,};

// ------------------------------------------------------------------------------
// Decode huffman codes and quantize it
// ------------------------------------------------------------------------------
static int decode_quantize(float xr[SBLIMIT][SSLIMIT],int *sf,
   struct gr_info *gi,int sfreq,int part2bits,struct frame *fr)
{
  	int shift = 1 + gi->scalefac_scale;
  	float *ptr = (float *) xr;
  	int hcodes = gi->part2_3_length - part2bits;
	int region[3] ={0,};
	register short *val;

	int count1;
    int i, cb = 0;
    float v = 0;
    int mc = 0, *me;
	int num;
  	long mask;

/*xxxxxxxxxxxxxxxxxxx------------------------0000000000000000000000000000
  |		    |			     |				|
  0		    big_values*2	     big_values*2+count1*4	576
 /		    \___________________________________________	 
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx|   step = 2
|		|		|				|	
0		region1		region2				2*big_values	
	These part contains the sound data  	*/
 
	if(gi->region1 > gi->region2)	// read error
		return 1;
	
    	if(gi->big_values <= gi->region1) 		//have 1 region
      		region[0] = gi->big_values; 
    	else 
	{
		region[0] = gi->region1;
      		if(gi->big_values <= gi->region2) 	//have 2 region
        		region[1] = gi->big_values - region[0]; 
      		else 					//have 3 region
		{
        		region[1] = gi->region2 - region[0]; 
			region[2] = gi->big_values - gi->region2;
      		}
    	}
	count1 = (288 - gi->big_values)>>1;
    
	num=getbitoffset();
  	mask = (long) getbits(num)<<(32-num);
  	hcodes -= num;

	if(gi->block_type == 2) 
	{
    		int step=0,lwin=3;
    		register int *m;

    		if(gi->mixed_block_flag) 
		{
      			m = map[sfreq][0];
      			me = mapend[sfreq][0];
    		}
    		else 
		{
     		 	m = map[sfreq][1];
      			me = mapend[sfreq][1];
    		}
    		for(i=0;i<2;i++) 
		{
      			struct newhuff *h = ht+gi->table_select[i];
      			for(;region[i];region[i]--,mc--) 
			{
        			register int x,y;
        			if( (!mc) ) 
				{
          				mc = *m++;
          				ptr = ((float *) xr) + (*m++);
          				lwin  = *m++;
          				cb = *m++;
          				if(lwin == 3) 
					{
            					v = gi->pow2gain[(*sf++) << shift];
            					step = 1;
          				}
          				else 
					{
            					v = gi->full_gain[lwin][(*sf++) << shift];
            					step = 3;
          				}
        			}
         	 		val = h->table;
          			REFRESH_MASK;
          			while((y=*val++)<0) 
				{
            				if (mask < 0)
              					val -= y;
            				num--;
            				mask <<= 1;
          			}
          			x = y >> 4;
          			y &= 0xf;

        			if(x == 15 && h->linbits) 
				{
          				REFRESH_MASK;
         			 	x += ((unsigned long) mask) >> (32-h->linbits);
          				num -= h->linbits+1;
          				mask <<= h->linbits;
          				if(mask < 0)
            					*ptr = -ispow[x] * v;
          				else
            					*ptr = ispow[x] * v;
          				mask <<= 1;
        			}
        			else if(x) 
				{
          				if(mask < 0)
            					*ptr = -ispow[x] * v;
          				else
            					*ptr = ispow[x] * v;
          				num--;
          				mask <<= 1;
        			}
        			else
          				*ptr = 0;

        			ptr += step;
        			if(y == 15 && h->linbits) 
				{
          				REFRESH_MASK;
          				y += ((unsigned long) mask) >> (32-h->linbits);
          				num -= h->linbits+1;
          				mask <<= h->linbits;
          				if(mask < 0)
            					*ptr = -ispow[y] * v;
          				else
            					*ptr = ispow[y] * v;
          				mask <<= 1;
        			}
        			else if(y) 
				{
          				if(mask < 0)
            					*ptr = -ispow[y] * v;
          				else
            					*ptr = ispow[y] * v;
          				num--;
          				mask <<= 1;
        			}
        			else
          				*ptr = 0;
        		ptr += step;
      		}
    	}
	// big value->count1
    	for(;count1 && (hcodes+num > 0);count1--) 
	{
      		register short* val = gi->count1table_select ? t1 : t0;
		register short a;

      		if(!(ptr < &xr[SBLIMIT][0]))	// for safe
			return 1;

	      	REFRESH_MASK;
      		while((a=*val++)<0) 
		{
        		if (mask < 0)
          			val -= a;
        		num--;
        		mask <<= 1;
      		}
      		if(hcodes+num <= 0) 
		{
			num -= hcodes+num;
			break;
      		}
      		for(i=0;i<4;i++) 
		{
        		if(!(i & 1)) 
			{
          			if(!mc) 
				{
            				mc = *m++;
            				ptr = ((float *) xr) + (*m++);
            				lwin = *m++;
            				cb = *m++;
            				if(lwin == 3) 
					{
              					v = gi->pow2gain[(*sf++) << shift];
              					step = 1;
            				}
            				else 
					{
              					v = gi->full_gain[lwin][(*sf++) << shift];
              					step = 3;
            				}
          			}
          			mc--;
        		}
        		if( (a & (0x8>>i)) ) 
			{
          			if(hcodes+num <= 0) 
            				break;
          			if(mask < 0) 
            				*ptr = -v;
          			else
            				*ptr = v;
          			num--;
          			mask <<= 1;
        		}
        		else
          			*ptr = 0;
        		ptr += step;
      		}
    	}

    	if(lwin < 3) 
      		while(1) 				// begin reorder
		{
        		for(;mc > 0;mc--) 
			{
          			*ptr = 0; ptr += 3; 	// short band -> step=3 
          			*ptr = 0; ptr += 3;
        		}
        		if(m >= me)
          			break;
        		mc = *m++;
        		ptr = ((float *) xr) + *m++;
        		if(*m++ == 0)
          			break; 
        		m++;
      		}	
}
else 
{
    	int *pretab = gi->preflag ? pretab1 : pretab2;
    	int *m = map[sfreq][2];
		register short *val;

    	for(i=0;i<3;i++) 
	{
      		struct newhuff *h = ht+gi->table_select[i];
      		for(;region[i];region[i]--,mc--) 
		{
        		int x,y;
        		if(!mc) 
			{
          			mc = *m++;
          			cb = *m++;
          			if(cb != 21)
            				v = gi->pow2gain[((*sf++) + (*pretab++)) << shift];
        		}
          		val = h->table;
          		REFRESH_MASK;
          		while((y=*val++)<0) 
			{
            			if (mask < 0)
              				val -= y;
            			num--;
            			mask <<= 1;
          		}
          		x = y >> 4;
          		y &= 0xf;

        		if(x == 15 && h->linbits) 
			{
	  			REFRESH_MASK;
          			x += ((unsigned long) mask) >> (32-h->linbits);
          			num -= h->linbits+1;
          			mask <<= h->linbits;
          			if(mask < 0)
            				*ptr++ = -ispow[x]*v;
          			else
            				*ptr++ = ispow[x]*v;
          			mask <<= 1;
        		}
        		else if(x) 
			{
          			if(mask < 0)
            				*ptr++ = -ispow[x]*v;
          			else
            				*ptr++ = ispow[x]*v;
          			num--;
          			mask <<= 1;
        		}
        		else
         			 *ptr++ = 0;

        		if (y == 15 && h->linbits) 
			{
	  			REFRESH_MASK;
          			y += ((unsigned long) mask) >> (32-h->linbits);
          			num -= h->linbits+1;
          			mask <<= h->linbits;
          			if(mask < 0)
            				*ptr++ = -ispow[y] * v;
          			else
            				*ptr++ = ispow[y] * v;
          			mask <<= 1;
        		}
        		else if(y) 
			{
          			if(mask < 0)
            				*ptr++ = -ispow[y] * v;
          			else
            				*ptr++ = ispow[y] * v;
          			num--;
          			mask <<= 1;
        		}
        		else
          			*ptr++ = 0;
      		}
    	}

    	for(;count1 && (hcodes+num > 0);count1--) 
	{
      		register short* val = gi->count1table_select ? t1 : t0;
		register short a;

      		REFRESH_MASK;
      		while((a=*val++)<0) 
		{
        		if (mask < 0)
          			val -= a;
        		num--;
        		mask <<= 1;
      		}
      		if(hcodes+num <= 0) 
		{
			num -= hcodes+num;
        		break;
      		}

      		for(i=0;i<4;i++) 
		{
        		if(!(i & 1)) 
			{
          			if(!mc) 
				{
            				mc = *m++;
            				cb = *m++;
            				if(cb != 21)
             			 		v = gi->pow2gain[((*sf++)+(*pretab++)) << shift];
          			}
          			mc--;
        		}	
        		if((a & (0x8>>i))) 
			{
          			if(hcodes+num <= 0) 
            				break;
          			if(mask < 0)
            				*ptr++ = -v;
          			else
            				*ptr++ = v;
          			num--;
          			mask <<= 1;
        		}
        		else
          			*ptr++ = 0;
      		}
    	}
}

  	hcodes += num;
  	backbits(num);
  	num = 0;
  	while(ptr < &xr[SBLIMIT][0]) 
    		*ptr++ = 0;
  	while(hcodes > 16) 
	{
    		getbits(16); 
    		hcodes -= 16;
  	}
  	if(hcodes > 0)
    		getbits(hcodes);
  	else if(hcodes < 0) 
    		return 1; 
  	return 0;
}
// ------------------------------------------------------------------------------
// DCT
// ------------------------------------------------------------------------------
static void dct36(float *inbuf,float *o1,float *o2,float *wintab,float *tsbuf)
{
  	float tmp[18];
	float *out2;
   	float *w;
   	float *out1;
	float *ts;
	int i;
	float t0, t1, t2, t3;

    	register float *in = inbuf;
    	in[17]+=in[16]; in[16]+=in[15]; in[15]+=in[14];
    	in[14]+=in[13]; in[13]+=in[12]; in[12]+=in[11];
    	in[11]+=in[10]; in[10]+=in[9];  in[9] +=in[8];
    	in[8] +=in[7];  in[7] +=in[6];  in[6] +=in[5];
    	in[5] +=in[4];  in[4] +=in[3];  in[3] +=in[2];
    	in[2] +=in[1];  in[1] +=in[0];
    	in[17]+=in[15]; in[15]+=in[13]; in[13]+=in[11]; in[11]+=in[9];
    	in[9] +=in[7];  in[7] +=in[5];  in[5] +=in[3];  in[3] +=in[1];

    	{	//compute temp[0-8]   		
      		t0 = (in[8]+in[16]-in[4])/2;
      		t1 = in[12]/2;
      		t3 = in[0];
      		t2 = t3 - t1 - t1;

      		tmp[1] = tmp[7] = t2-t0;
      		tmp[4] = t2+t0+t0;
      		t3 += t1;
      		t2 = (in[10]+in[14]-in[2])*0.866025;	//cos(pi/6)
      		tmp[1] -= t2;
      		tmp[7] += t2;
      		t0 =(in[4]+in[8] )*0.939693;		//cos(pi/9)
     		t1 =(in[8]-in[16])*(-0.173648);		//cos(5pi/9)
      		t2 =(in[4]+in[16])*(-0.766044);		//cos(7pi/9)
      		tmp[2] = tmp[6] = t3-t0-t2;
      		tmp[0] = tmp[8] = t3+t0+t1;
      		tmp[3] = tmp[5] = t3-t1+t2;
      		t1 = (in[2]+in[10])*cos(PI/18);
      		t2 = (in[10]-in[14])*cos(11*PI/18);
      		t3 = in[6]*cos(PI/6);
        	t0 = t1 + t2 + t3;
        	tmp[0] += t0;
        	tmp[8] -= t0;

      		t2 -= t3;
      		t1 -= t3;
      		t3 = (in[2] + in[14])*cos(13*PI/18);
      		t1 += t3;
      		tmp[3] += t1;
      		tmp[5] -= t1;
      		t2 -= t3;
      		tmp[2] += t2;
      		tmp[6] -= t2;
    	}
    	{	//compute tmp[9-17]
      		float t4, t5, t6, t7;
      		t1 = in[13]/2;
      		t2 = (in[9]+in[17]-in[5])/2;
      		t3 = in[1]+t1;
      		t4 = in[1]-t1-t1;
      		t5 = t4-t2;
      		t0 = (in[5]+in[9])*0.939693;
      		t1 = (in[9]-in[17])*(-0.173648);

      		tmp[13] = (t4 + t2 + t2)/cos(PI/4)/2;	//didn't effect the sound quality at all
      		t2 = (in[5] + in[17])*(-0.766044);
      		t6 = t3 - t0 - t2;
      		t0 += t3 + t1;
      		t3 += t2 - t1;
      		t2 = (in[3]  + in[11])*cos(PI/18);
      		t4 = (in[11] - in[15])*cos(11*PI/18);
      		t7 = in[7]*cos(PI/6);
      		t1 = t2 + t4 + t7;
      		tmp[17] = (t0 + t1)/cos(PI/36)/2;
      		tmp[9]  = (t0 - t1)/cos(17*PI/36)/2;
      		t1 = (in[3] + in[15])*cos(13*PI/18);
      		t2 += t1 - t7;

      		tmp[14] = (t3 + t2)/cos(7*PI/36)/2;
      		t0 = (in[11] + in[15] - in[3])*cos(PI/6);
      		tmp[12] = (t3 - t2)/cos(11*PI/36)/2;
      		t4 -= t1 + t7;
      		tmp[16] = (t5 - t0)/cos(3*PI/36)/2;
      		tmp[10] = (t5 + t0)/cos(15*PI/36)/2;
      		tmp[15] = (t6 + t4)/cos(5*PI/36)/2;
      		tmp[11] = (t6 - t4)/cos(13*PI/36)/2;
   	}
	out2 = o2;
   	w = wintab;
   	out1 = o1;
	ts = tsbuf;

	for(i=0;i<9;i++)		//output0-18
	{ 
		float tmpval;
		tmpval = tmp[i] + tmp[17-i]; 
		out2[9+i] =(tmpval*w[27+i]); 
		out2[8-i] =(tmpval*w[26-i]); 
		tmpval = tmp[i] - tmp[17-i]; 
		ts[SBLIMIT*(8-i)] = out1[8-i] +(tmpval*w[8-i]); 
		ts[SBLIMIT*(9+i)] = out1[9+i] +(tmpval*w[9+i]); 
	}
}
// ------------------------------------------------------------------------------
// DCT
// ------------------------------------------------------------------------------
static void dct12(float *in,float *rawout1,float *rawout2,float *wi,float *ts)
{
#define DCT12_PART1 \
             in5 = in[5*3];  \
     in5 += (in4 = in[4*3]); \
     in4 += (in3 = in[3*3]); \
     in3 += (in2 = in[2*3]); \
     in2 += (in1 = in[1*3]); \
     in1 += (in0 = in[0*3]); \
     in5 += in3; in3 += in1; \
     in2 = in2*cos(PI/6); \
     in3 = in3*cos(PI/6); \

#define DCT12_PART2 \
     in0 += in4/2; \
                          \
     in4 = in0 + in2;     \
     in0 -= in2;          \
                          \
     in1 += in5/2; \
                          \
     in5 = (in1 + in3)*cos(PI/12)/2; \
     in1 = (in1 - in3)*cos(5*PI/12)/2; \
                         \
     in3 = in4 + in5;    \
     in4 -= in5;         \
                         \
     in2 = in0 + in1;    \
     in0 -= in1;

	float in0,in1,in2,in3,in4,in5;
	float *out1 = rawout1;
 	float *out2 = rawout2;
	float tmp0, tmp1, tmp2;
   	{    			
    	 	ts[SBLIMIT*0] = out1[0]; ts[SBLIMIT*1] = out1[1]; ts[SBLIMIT*2] = out1[2];
     		ts[SBLIMIT*3] = out1[3]; ts[SBLIMIT*4] = out1[4]; ts[SBLIMIT*5] = out1[5];
 
     		DCT12_PART1

       		tmp1 = (in0 - in4);
         	tmp2 = (in1 - in5)*cos(PI/4)/2;
         	tmp0 = tmp1 + tmp2;
         	tmp1 -= tmp2;

       		ts[(17-1)*SBLIMIT] = out1[17-1] + (tmp0*wi[11-1]);
       		ts[(12+1)*SBLIMIT] = out1[12+1] + (tmp0*wi[6+1]);
       		ts[(6 +1)*SBLIMIT] = out1[6 +1] + (tmp1*wi[1]);
       		ts[(11-1)*SBLIMIT] = out1[11-1] + (tmp1*wi[5-1]);

     		DCT12_PART2

     		ts[(17-0)*SBLIMIT] = out1[17-0] + (in2*wi[11-0]);
     		ts[(12+0)*SBLIMIT] = out1[12+0] + (in2*wi[6+0]);
     		ts[(12+2)*SBLIMIT] = out1[12+2] + (in3*wi[6+2]);
     		ts[(17-2)*SBLIMIT] = out1[17-2] + (in3*wi[11-2]);
     		ts[(6 +0)*SBLIMIT] = out1[6+0] + (in0*wi[0]);
     		ts[(11-0)*SBLIMIT] = out1[11-0] + (in0*wi[5-0]);
     		ts[(6 +2)*SBLIMIT] = out1[6+2] + (in4*wi[2]);
     		ts[(11-2)*SBLIMIT] = out1[11-2] + (in4*wi[5-2]);
	}

  	in++;
  	{
     		DCT12_PART1

     		tmp1 = (in0 - in4);
       		tmp2 = (in1 - in5)*cos(PI/4)/2;
         	tmp0 = tmp1 + tmp2;
         	tmp1 -= tmp2;
       		
       		out2[5-1] = (tmp0*wi[11-1]);
       		out2[0+1] = (tmp0*wi[6+1]);
       		ts[(12+1)*SBLIMIT] += (tmp1*wi[1]);
       		ts[(17-1)*SBLIMIT] += (tmp1*wi[5-1]);
     	
     		DCT12_PART2

     		out2[5-0] = (in2*wi[11-0]);
     		out2[0+0] = (in2*wi[6+0]);
     		out2[0+2] = (in3*wi[6+2]);
    		out2[5-2] = (in3*wi[11-2]);

     		ts[(12+0)*SBLIMIT] += (in0*wi[0]);
     		ts[(17-0)*SBLIMIT] += (in0*wi[5-0]);
     		ts[(12+2)*SBLIMIT] += (in4*wi[2]);
     		ts[(17-2)*SBLIMIT] += (in4*wi[5-2]);
  	}

  	in++; 

  	{
     		out2[12]=out2[13]=out2[14]=out2[15]=out2[16]=out2[17]=0.0;

     		DCT12_PART1
     
       		tmp1 = (in0 - in4);
         	tmp2 = (in1 - in5)*cos(PI/4)/2;
         	tmp0 = tmp1 + tmp2;
         	tmp1 -= tmp2;

      		out2[11-1] = (tmp0*wi[11-1]);
       		out2[6 +1] = (tmp0*wi[6+1]);
      		out2[0+1] += (tmp1*wi[1]);
       		out2[5-1] += (tmp1*wi[5-1]);
     	
     		DCT12_PART2

     		out2[11-0] = (in2*wi[11-0]);
     		out2[6 +0] = (in2*wi[6+0]);
     		out2[6 +2] = (in3*wi[6+2]);
     		out2[11-2] = (in3*wi[11-2]);

     		out2[0+0] += (in0*wi[0]);
     		out2[5-0] += (in0*wi[5-0]);
     		out2[0+2] += (in4*wi[2]);
     		out2[5-2] += (in4*wi[5-2]);
  	}
}
// ------------------------------------------------------------------------------
// hybrid 
// ------------------------------------------------------------------------------
static void hybrid(float is[SBLIMIT][SSLIMIT],float xr[SSLIMIT][SBLIMIT],
   int ch,struct gr_info *gi)
{
   	static float block[2][2][SBLIMIT*SSLIMIT] = {{{0,}}};
   	static int blc[2]={0,0};
   	float *tspnt = (float *) xr;
   	float *rawout1,*rawout2;
   	int sb = 0;
	int i;
	
     	int b = blc[ch];
     	rawout1 = block[b][ch];
     	b = -b+1;
     	rawout2 = block[b][ch];
     	blc[ch] = b;

   	if(gi->mixed_block_flag) 
	{
     		sb = 2;
     		dct36(is[0],rawout1,rawout2,win[0],tspnt);
     		dct36(is[1],rawout1+18,rawout2+18,win1[0],tspnt+1);
     		rawout1 += 36; rawout2 += 36; tspnt += 2;
   	}
   	if(gi->block_type == 2) 
	{
     		for (; sb<32; sb+=2,tspnt+=2,rawout1+=36,rawout2+=36) 
		{
       			dct12(is[sb], rawout1, rawout2, win[2], tspnt);
       			dct12(is[sb+1],rawout1+18,rawout2+18,win1[2],tspnt+1);
     		}
   	}
   	else 
	{
     		for (; sb<32; sb+=2,tspnt+=2,rawout1+=36,rawout2+=36) 
		{
       			dct36(is[sb],rawout1,rawout2,win[gi->block_type],tspnt);
       			dct36(is[sb+1],rawout1+18,rawout2+18,win1[gi->block_type],tspnt+1);
     		}
   	}
	//overlap data
	
   	for(;sb<SBLIMIT;sb++,tspnt++) 
     		for(i=0;i<SSLIMIT;i++) 
		{
       			tspnt[i*SBLIMIT] = *rawout1++;
       			*rawout2++ = 0;
     		}
}
#if 1
//alias when gr_info->block_type != 2.
static void alias(float xr[SBLIMIT][SSLIMIT],struct gr_info *gi) 
{
	float *xr1;
	int sb,i;
   	if(gi->block_type == 2)
      		return; 
	sb = 1;
	i = 0;
    xr1=(float *) xr[1];
	for(;sb<32;sb++,xr1+=10)
	{
       		float *csptr=cs,*captr=ca;
       		float *xr2 = xr1;
       		for(;i<8;i++)
       		{
        		 float bu = *--xr2, bd = *xr1;
        		*xr2 = bu*(*csptr) - bd*(*captr);
        		*xr1++ = bd*(*csptr++) + bu*(*captr++);
       		}
     	}
}
#endif
// ------------------------------------------------------------------------------ 
//	handle the layer3 sound data, then play granule by granule
// ------------------------------------------------------------------------------ 
void yyj_do_layer3(struct frame *fr,unsigned char *pcm_buf)
{
  	int gr, ch, ss;
  	int scalefacs[2][39]={{0,}};
  	struct sideinfo si;
  	int stereo = fr->stereo;
  	int sfreq = fr->sampling;
  	int granules=2;
	int i;


	g_pcm_sample=pcm_buf;
	g_pcm_point=0;



  	if(fr->lsf)//1-MPEG1.0, 0-MPEG2.0
    		granules = 1;
	
  	if(get_side_info(&si,stereo,sfreq,fr->lsf,fr))	// get side info
		return;
  	set_pointer(si.main_data_begin);		// reset bitstream
	//<---------Check Success! September 14. 4:24					
  	for (gr=0;gr<granules;gr++) 
	{
    	float is[2][SBLIMIT][SSLIMIT]={{{0,}}};
    	float xr[2][SSLIMIT][SBLIMIT]={{{0,}}};
		for(ch=0;ch<stereo;ch++)
    	{
      		struct gr_info *gi = &(si.ch[ch].gr[gr]);
      		long part2_len = get_scale_factors(scalefacs[ch],gi);//<--Success Sep14.
			// There seems exist some fault..  fixed it Sep 18. 11:20
      		decode_quantize(is[ch], scalefacs[ch],gi,sfreq,part2_len,fr);
		}		
#if 1		// MS-stereo handler
		i=0;
		if((fr->mode==1) && ((fr->mode_ext&2)==2))	
        	for(;i<576;i++) 
			{
        	  	((float*)is[0])[i]=0.7071*(((float*)is[0])[i]+((float*)is[1])[i]);
				((float*)is[1])[i]=0.7071*(((float*)is[0])[i]-((float*)is[1])[i]);
        	}
#endif
		// i-stereo handler
		//if((fr->mode==1) && ((fr->mode_ext&1)==1))
		//	printf("This is a song with i-stereo!\n");
		for(ch=0;ch<stereo;ch++)
		{
			struct gr_info *gi = &(si.ch[ch].gr[gr]);
			alias(is[ch],gi);		
      		hybrid(is[ch], xr[ch], ch,gi);
    	}			
		// the sound data is stored in xr. I must synthesize it using MDCT.
    	for(ss=0;ss<SSLIMIT;ss++) 
		{
        	int p1 = pcm_point;
        	//synthesize(xr[0][ss],0,g_pcm_sample,&p1);
        	//synthesize(xr[1][ss],1,g_pcm_sample,&pcm_point);

			synthesize(xr[0][ss],0,g_pcm_sample,&p1);
        	synthesize(xr[1][ss],1,g_pcm_sample,&pcm_point);
			
		
			g_pcm_sample+=pcm_point;
			g_pcm_point+=pcm_point;

			pcm_point=0;
    	}
  	}
}
