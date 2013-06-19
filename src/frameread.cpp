// ==============================================================================
//	Last modification of the initial version.
//					--September 8 5:34pm
// ==============================================================================
#include "mp.h"
#include "./dec_frame/frame.h"

#define callback

/* bitrates: [mpeg1/2][layer][bitrate_index] */
int br_index[2][3][16] = {
   { {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,},
     {0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,},
     {0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,} },

   { {0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,},
     {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,},
     {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,} }
};
unsigned freqs[9] = { 44100, 48000, 32000, 22050, 24000, 16000 , 11025 , 12000 , 8000 };
struct bit_info bi;
static int fsizeold=0,ssize;
static unsigned char bsspace[2][3072]; // one for cur, one for old
unsigned char *bsbuf=bsspace[1],*bsbufold;
static int bsnum=0;
static unsigned long oldhead = 0;
//unsigned long firsthead=0;
extern bool issupported;

// ------------------------------------------------------------------------------
// Read the file
//	return value:
//		-1    - failure
//		other - success	
// ------------------------------------------------------------------------------
int fullread(struct reader *rds,unsigned char *buf,int count)
{
  	int ret,cnt=0;

  	if ((rds->flags&2) && (rds->filepos+count > rds->filelen))
    		count = rds->filelen - rds->filepos;

  	while(cnt < count) 
	{
    		//ret = read(rds->filept,buf+cnt,count-cnt);
		ret =fread(buf+cnt,1,count-cnt,rds->filept);
	
    		if(ret < 0)
      			return -1;
    		if(ret == 0)
      			break;
    		rds->filepos += ret;

    		cnt += ret;
  	} 
  	return cnt;
}

// ------------------------------------------------------------------------------
// read frame head
//	return value:
//		0 - failure
//		1 - success
// ------------------------------------------------------------------------------
int frame_head_read(struct reader *rds,unsigned long *newhead)
{
  	unsigned char headbuf[65535];
  	if(fullread(rds,headbuf,4) != 4)
    		return 0;  
	*newhead = (headbuf[0]<<24) | (headbuf[1]<<16) | (headbuf[2]<<8) | headbuf[3];

	return 1;
}
// ------------------------------------------------------------------------------
// read frame body into bsbuf
//	return value: 
//		0 - failure
//		1 - success
// ------------------------------------------------------------------------------
int frame_body_read(struct reader *rds,unsigned char *buf,  int size)
{
	int tmp; 
	if((tmp=fullread(rds,buf,size)) != size)
  		if(tmp == -1)
   			return 0;
  	return 1;
}

struct reader *rd;
struct reader readers[] ={{0,}};
// ------------------------------------------------------------------------------
// open frame
//	return value:
//		-1    - failure
//		other - success
// ------------------------------------------------------------------------------
FILE *file_open(char *bs_filenam)
{
	int filept_opened = 1;
	FILE *filept;
	filept=fopen(bs_filenam,"rb");
	if(filept==NULL)
	{
		printf("file open fail2:%s\r\n",bs_filenam);
		return 0;
	}
      		readers[0].filelen = -1;
      		readers[0].filept  = filept;
      		readers[0].flags = 0;
      		if(filept_opened)
        		readers[0].flags |= 1;	
			
		(readers+0)->filelen = fseek((readers+0)->filept,0,SEEK_END);
		(readers+0)->filepos = fseek((readers+0)->filept,0,SEEK_SET);        		
        	rd = &readers[0];

    	return filept;
}
// ------------------------------------------------------------------------------
// decode the header according to head definition
//	return value: 	0 - (no sync word; version not support; bit rate error)
//			1 - success 
// ------------------------------------------------------------------------------
static int decode_header(struct frame *fr,unsigned long newhead)
{
    	if((newhead&0xffe00000)==0xffe00000)	// 11bit sync_word
		fr->sync = 1;
	else
	{
		fr->sync = 0;
		//printf("Didn't find the sync word!\n");
		return 0;
	}

    	if( newhead & (1<<20) )					// 1 MPEG1.0 or 2.0
      		fr->lsf = (newhead & (1<<19)) ? 0x0 : 0x1;	// 1-MPEG1.0, 0-MPEG2.0
    	else 
	{
	//	printf("MPEG 2.5 or reserved MPEG version!\n");
		return 0;
	}
	fr->layer = (newhead>>17)&3;		
	fr->protection = ((newhead>>16)&1)^1;	
	fr->bitrate = (newhead>>12)&0xf;	
	fr->sampling = ((newhead>>10)&3) + (fr->lsf*3);
    	fr->padding   = (newhead>>9)&1;
    	fr->privat = (newhead>>8)&1;
    	fr->mode      = (newhead>>6)&3;
    	fr->mode_ext  = (newhead>>4)&3;
    	fr->copyright = (newhead>>3)&1;
    	fr->original  = (newhead>>2)&1;
    	fr->emphasis  = newhead & 3;

    	fr->stereo = (fr->mode == 3) ? 1 : 2;
    	oldhead = newhead;

	/*printf("bitrate:%d\n",fr->bitrate);//128k
	printf("sampling:%d\n",fr->sampling);//44100
	printf("stereo:%d\n",fr->stereo);
	printf("lsf:%d\n",fr->lsf);*/
	if(fr->bitrate!=9&&fr->bitrate!=12)
	{
		#if GSVA_PROTECT
		EnterCriticalSection(&CriticalSection);
		#endif
		issupported=FALSE;
		isfinished=TRUE;
		#if GSVA_PROTECT
		LeaveCriticalSection(&CriticalSection);
		#endif

		return 0;
	}
    	if(!fr->bitrate) 
		{
      		printf("Reading bit rate error!\n");
      		return 0;
    	}

       	ssize = (fr->stereo == 1) ? 17 : 32;
        if(fr->protection)
          	ssize += 2;
        fr->size  = br_index[fr->lsf][2][fr->bitrate]*(fr->lsf?72000:144000);
        fr->size /= freqs[fr->sampling]<<(fr->lsf);
        fr->size = fr->size + fr->padding - 4;
    	return 1;
}
// ------------------------------------------------------------------------------
// skips the ID3 header at the beginning
//	return value:
//		0     - read-error
//        	other - skipping succeeded	
// ------------------------------------------------------------------------------				
void skip_id3v2(struct reader *rds)
{
	unsigned char labelheadbuf1[4];
	unsigned char labelheadbuf2[6];
	unsigned long newhead;
	unsigned long length=0;
	int ret;

	fseek(rds->filept, 0, SEEK_SET);
	
	// read the first 4 bytes in id3v2, then compare them with "ID3"
	ret = fread(labelheadbuf1,1,4,rds->filept);
	if(ret!=4)
	{
		printf("read id3v2 label head error!\n");
		return ;
	}
	rds->filepos += ret;
  
	newhead = (labelheadbuf1[0]<<24) | (labelheadbuf1[1]<<16) | (labelheadbuf1[2]<<8) | labelheadbuf1[3];
	if((newhead & 0xffffff00) != 0x49443300) //"ID3" tags, symbol of begin
	{
		printf("no ID3 tag in label head!\n");
		return ;
	}

	// read 6 bytes in id3v2 label head, then compute the size of id3v2
	ret = fread(labelheadbuf2,1,6,rds->filept);
	if(ret!=6)
	{
		printf("read id3v2 label head error!\n");
		return ;
	}
	rds->filepos += ret;
	
	length = ((labelheadbuf2[2]&0x7f)<<27)|((labelheadbuf2[3]&0x7f)<<14)|((labelheadbuf2[4]&0x7f)<<7)|(labelheadbuf2[5]&0x7f);
	fseek(rds->filept, length, SEEK_CUR);
	rds->filepos += length;
	
	return ;
}
// ------------------------------------------------------------------------------ 
// read frame
//	return 0 - read frame failure(header inlege or frame body error or )
//	return 1 - success. 
// ------------------------------------------------------------------------------
int frame_read(struct frame *fr)
{
  	unsigned long newhead;

  	fsizeold=fr->size;
read_again:
	if(!frame_head_read(rd,&newhead))
		return 0;
	
    	fr->header_change = 2;
 
      	if(!decode_header(fr,newhead))
       	goto read_again;
	
  	bsbufold = bsbuf;
  	bsbuf = bsspace[bsnum]+512; // main data begin = 511 max.
  	bsnum = (bsnum + 1) & 1;
	if(!frame_body_read(rd,bsbuf,fr->size))
		return 0;

  	bi.index = 0;		//set the bit stream info, reading main sound data
  	bi.ptr = (unsigned char *) bsbuf;

  	return 1;
}
// ------------------------------------------------------------------------------
// set the bit index position 
// ------------------------------------------------------------------------------
void callback set_pointer(long backstep)
{
  	bi.ptr = bsbuf + ssize - backstep;
  	if(backstep)	// main data begin 
    		memcpy(bi.ptr,bsbufold+fsizeold-backstep,backstep);
  	bi.index = 0; 
}
