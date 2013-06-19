// PlayWav_Dos.cpp : Defines the entry point for the console application.
//

#include "mp.h"

    
FILE *fp_wav;

PCMWAVEFORMAT pcmwf;

u16 actual_rd_data_length1;
u16 actual_rd_data_length2;
u16 actual_rd_data_length3;
u32 data_size;
bool iswav=FALSE;

static void prepare_wave_header()
{
	pWaveHdr1->lpData            = (char*)pdata1;
	pWaveHdr1->dwBufferLength    = ONCE_RD_DATA_LENGTH;
	pWaveHdr1->dwBytesRecorded   = 0;   
	pWaveHdr1->dwUser            = 0;   
	pWaveHdr1->dwFlags           = 0;   
	pWaveHdr1->dwLoops           = 1;   
	pWaveHdr1->lpNext            = NULL;
	pWaveHdr1->reserved          = 0;
	waveOutPrepareHeader(hWaveOut,pWaveHdr1,sizeof(WAVEHDR));

	pWaveHdr2->lpData            = (char*)pdata2;
	pWaveHdr2->dwBufferLength    = ONCE_RD_DATA_LENGTH;
	pWaveHdr2->dwBytesRecorded   = 0;   
	pWaveHdr2->dwUser            = 0;   
	pWaveHdr2->dwFlags           = 0;   
	pWaveHdr2->dwLoops           = 1;   
	pWaveHdr2->lpNext            = NULL;
	pWaveHdr2->reserved          = 0;
	waveOutPrepareHeader(hWaveOut,pWaveHdr2,sizeof(WAVEHDR));
}

void CALLBACK wavWaveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance,
       DWORD dwParam1, DWORD dwParam2)
{
	switch(uMsg) 
	{
		case WOM_OPEN:  // connection opened
			//printf("opened\n");
			//waveOutWrite(hWaveOut, pWaveHdr1, sizeof(WAVEHDR));//can not play wave here as waveOutPrepareHeader is not finished
			
			break;
		case WOM_DONE:  // buffer finished playing
			//printf("done one frame\n");
			{

			PWAVEHDR pWaveHeader = (PWAVEHDR)dwParam1;//系统自动识别是哪一个WAVEHDR播放完毕
			//waveOutUnprepareHeader(hWaveOut, pWaveHeader, sizeof(WAVEHDR) );//播放完后须调用此函数
			//printf("isfinished=%d\r\n",isfinished);
			point_index++;
			if(point_index%50==0)
				printf("\r\n");
			else
				printf(".");
	       	if (!isfinished)
			{
				actual_rd_data_length3=fread(pWaveHeader->lpData,1,ONCE_RD_DATA_LENGTH,fp_wav);
				if (actual_rd_data_length3<ONCE_RD_DATA_LENGTH)
				{
					pWaveHeader->dwBufferLength    = actual_rd_data_length3;
					#if GSVA_PROTECT
					EnterCriticalSection(&CriticalSection);
					#endif
					isfinished=TRUE;
					#if GSVA_PROTECT
					LeaveCriticalSection(&CriticalSection);
					#endif

					//fseek(fp_wav,-(data_size-4),SEEK_END);
					break;
				}
				else
				{
				    pWaveHeader->dwBufferLength    = ONCE_RD_DATA_LENGTH;		
				}
			
				waveOutWrite(hWaveOut, pWaveHeader, sizeof(WAVEHDR));
			}
			}
			break;
		case WOM_CLOSE:  // connection closed
			//AfxMessageBox("closed");
			break;
	}
}


void sound_play()
{
	if(iswav)//wav file param
	{
		waveformat.wFormatTag        = WAVE_FORMAT_PCM;
		waveformat.nChannels         = pcmwf.wf.nChannels;
		waveformat.nSamplesPerSec    = pcmwf.wf.nSamplesPerSec;
		waveformat.nAvgBytesPerSec   = pcmwf.wf.nAvgBytesPerSec;
		waveformat.nBlockAlign       = pcmwf.wf.nBlockAlign;
		waveformat.wBitsPerSample    = pcmwf.wBitsPerSample;
		waveformat.cbSize            = 0;
	}
	else//pcm file param
	{
		waveformat.wFormatTag        = WAVE_FORMAT_PCM;
	    waveformat.nChannels         = 2;//pcmwf.wf.nChannels;
		waveformat.nSamplesPerSec    = 44100;//pcmwf.wf.nSamplesPerSec;
		waveformat.nAvgBytesPerSec   = 44100*4;//pcmwf.wf.nAvgBytesPerSec;
		waveformat.nBlockAlign       = 4;//pcmwf.wf.nBlockAlign;
		waveformat.wBitsPerSample    = 16;//pcmwf.wBitsPerSample;
		waveformat.cbSize            = 0;
	}
                      
    if(waveOutOpen(&hWaveOut,WAVE_MAPPER,&waveformat,(DWORD)wavWaveOutProc,0,CALLBACK_FUNCTION)!=MMSYSERR_NOERROR)   
    {                                              //MyWaveOutProc  //CALLBACK_FUNCTION
		//fprintf(fp_log,"waveout open fail\r\n");
		free(pWaveHdr1);		
        hWaveOut = NULL;
        return;
    }

	//AfxMessageBox("open is finished!");
	prepare_wave_header();


	actual_rd_data_length1=fread(pdata1,1,ONCE_RD_DATA_LENGTH,fp_wav);
	if (actual_rd_data_length1<ONCE_RD_DATA_LENGTH)
	{
		#if GSVA_PROTECT
		EnterCriticalSection(&CriticalSection);
		#endif
		isfinished=TRUE;
		#if GSVA_PROTECT
		LeaveCriticalSection(&CriticalSection);
		#endif

		//fseek(fp_wav,-(data_size-4),SEEK_END);
		pWaveHdr1->dwBufferLength    = actual_rd_data_length1;		
	}
	waveOutWrite(hWaveOut, pWaveHdr1, sizeof(WAVEHDR));


	actual_rd_data_length2=fread(pdata2,1,ONCE_RD_DATA_LENGTH,fp_wav);
	if (actual_rd_data_length2<ONCE_RD_DATA_LENGTH)
	{
		#if GSVA_PROTECT
		EnterCriticalSection(&CriticalSection);
		#endif
		isfinished=TRUE;
		#if GSVA_PROTECT
		LeaveCriticalSection(&CriticalSection);
		#endif

		//fseek(fp_wav,-(data_size-4),SEEK_END);
		pWaveHdr2->dwBufferLength    = actual_rd_data_length2;	
	}
	waveOutWrite(hWaveOut, pWaveHdr2, sizeof(WAVEHDR));
}

void parseWavHeader(char *filename)
{
	fp_wav=fopen(filename,"rb");
	if(NULL==fp_wav)
	{
		printf("open wav fail\r\n");
		return;
	}

	u32 riff_flag;
	fread(&riff_flag,1,4,fp_wav);
	if (riff_flag==0x46464952)		//ffir->FFIR
	{
		printf("riff wav file\r\n");
		iswav=TRUE;//is wav file

		u32 data_length;
		fread(&data_length,1,4,fp_wav);
		//file size=riff[4]+data_length[4]+data[data_length]
		printf("the data length of the wav file is:%d\r\n",data_length);

		u32 wave_flag;
		fread(&wave_flag,1,4,fp_wav);
		if (wave_flag!=0x45564157)		//wave->EVAW
		{
			printf("not wave flag\r\n");
			return;
		}

		u32 fmt_flag;
		fread(&fmt_flag,1,4,fp_wav);
		if (fmt_flag!=0x20746d66)		//fmt->tmf
		{
			printf("not wave flag\r\n");
			return;
		}

		u32 pcmwf_size;
		fread(&pcmwf_size,1,4,fp_wav);
		if (pcmwf_size!=16)
		{
			printf("the pcmwf length wrong, not 16 bytes\r\n");
			return;
		}

		//PCMWAVEFORMAT pcmwf;
		fread(&pcmwf,1,16,fp_wav);//sizeof(PCMWAVEFORMAT)=20

		u32 data_flag;
		fread(&data_flag,1,4,fp_wav);
		if (data_flag!=0x61746164)		//data->atad
		{
			printf("not data flag\r\n");
			return;
		}

		//u32 data_size;
		fread(&data_size,1,4,fp_wav);
		//file size=36+data chunk size[data_size]
		printf("the data chunk size of the wav file is:%d\r\n",data_size);
	}
	else
	{
		printf("pcm file\r\n");
		iswav=FALSE;//is pcm file
		fseek(fp_wav,0,SEEK_SET);
	}
}

