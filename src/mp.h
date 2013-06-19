#ifndef __MP_H__
#define __MP_H__

#include "../src/inc/stdio.h"
#include "../src/inc/conio.h"
#include "../src/inc/stdlib.h"
#include "../src/inc/string.h"
#include "../src/inc/math.h"
#include "../src/inc/windows.h"
#include  "../src/inc/mmsystem.h"
#include "../src/inc/ctype.h"
//#define _MT
#include "../src/inc/process.h"
#include "./dec_frame/frame.h"

#define	AUDIOBUFSIZE	16384
#define   SAMPLE_RATE           44100   
#define ONCE_RD_DATA_LENGTH 30000

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

struct reader
{
  	long filelen;
  	long filepos;
  	FILE *filept;		//fd
  	int  flags;		//file read flag
};

typedef struct tagID3V1
{
	char Header[3];    /*标签头必须是"TAG"否则认为没有标签*/
	char Title[30];    /*标题*/
	char Artist[30];   /*作者*/
	char Album[30];    /*专集*/
	char Year[4];    /*出品年代*/
	char Comment[28];   /*备注*/
	char reserve;      /*保留*/
	char track;; /*音轨*/
	char Genre;    /*类型*/
}ID3V1,*pID3V1;

extern struct reader *rd,readers[];
extern HWAVEOUT       hWaveOut;   
extern PWAVEHDR       pWaveHdr1,pWaveHdr2;   
extern WAVEFORMATEX   waveformat;   
extern u8 *pdata1;
extern u8 *pdata2;
extern bool isfinished;
extern int point_index;

//Global share variable access protect
#define GSVA_PROTECT 1 //Global share variable access
#if GSVA_PROTECT
extern CRITICAL_SECTION CriticalSection; 
#endif


extern FILE *file_open(char *);
extern int frame_read(struct frame *fr);
extern int frame_body_read(struct reader *,unsigned char *,int);
extern int frame_head_read(struct reader *rds,unsigned long *);
extern void play_frame(struct frame *fr);
extern void skip_id3v2(struct reader * rds);

extern void yyj_init_layer3(void);
extern void yyj_do_layer3(struct frame *fr,unsigned char *pcm_buf);
extern void ReadAllFiles(char *lpszDir);
extern void parseWavHeader(char *filename);
extern void sound_play();
extern void CALLBACK wavWaveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance,DWORD dwParam1, DWORD dwParam2);
extern void prepare_wave_header();
extern void parseID3V1(FILE *fp);
extern void freerscs();

#endif