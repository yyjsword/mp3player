#ifndef __FRAME_HEAD_H__
#define __FRAME_HEAD_H__

#define callback

#define SBLIMIT         32
#define SSLIMIT         18
#define	PI 		3.14159265358979

// 4   bytes
struct frame {
	int sync;		// 1 with sync_word, 0 without;
    	int lsf; 		
    	int layer;		
    	int protection;		
    	int bitrate;
    	int sampling;
    	int padding;
    	int privat;
    	int mode;		
    	int mode_ext;
    	int copyright;
    	int original;
    	int emphasis;		// defined in ISO 11172-3

        int header_change;
    	int size; 		// computed framesize 
	int stereo; 		// 1-mono, 2-stereo 
};

struct gr_info {
      	int scfsi;
      	unsigned part2_3_length;
      	unsigned big_values;
      	unsigned scalefac_compress;
      	unsigned block_type;
      	unsigned mixed_block_flag;
      	unsigned table_select[3];
      	unsigned subblock_gain[3];
      	unsigned region1;	//modified region1 address
      	unsigned region2;	//modified region2 address
      	unsigned preflag;	
      	unsigned scalefac_scale;
      	unsigned count1table_select;

      	float *pow2gain;
	float *full_gain[3];
	unsigned maxb;
};

struct sideinfo{
  	unsigned main_data_begin;
 	unsigned private_bits;
  	struct{
    		struct gr_info gr[2];	//two granules
  	}ch[2];				//two channel
};

struct bandInfoStruct {
  	int longIdx[23];
  	int longDiff[22];
  	int shortIdx[14];
  	int shortDiff[13];
};


struct bit_info 
{
  	int index;
  	unsigned char *ptr;
};

extern void callback set_pointer(long);
#endif
