#include "mp.h"

#define MP3_SUPPORT 1
#define WAV_SUPPORT 1
#define MULTI_THREAD 1

extern unsigned freqs[];
extern struct bit_info bi;
struct frame fr;
unsigned char *g_pcm_sample;
int g_pcm_point = 0;
FILE *fp_pcm;

HWAVEOUT       hWaveOut;   
PWAVEHDR       pWaveHdr1,pWaveHdr2;   
WAVEFORMATEX   waveformat;   
u8 *pdata1;
u8 *pdata2;
bool isfinished=FALSE;
u8 *pframe1;
u8 *pframe2;
int point_index=0;
char mp3files[MAX_PATH][MAX_PATH];
char wavfiles[MAX_PATH][MAX_PATH];
int mp3fileNum=0;
int wavfileNum=0;
int mp3index;
int wavindex;
bool isloop=FALSE;
bool issupported=TRUE;
volatile bool isexit=FALSE;
#if GSVA_PROTECT
CRITICAL_SECTION CriticalSection; 
#endif

void CALLBACK mp3WaveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance,
								DWORD dwParam1, DWORD dwParam2)
{
	switch(uMsg) 
	{
		case WOM_OPEN:  // connection opened
			//AfxMessageBox("opened\n");
			//waveOutWrite(hWaveOut, pWaveHdr1, sizeof(WAVEHDR));//can not play wave here as waveOutPrepareHeader is not finished
			break;
		case WOM_DONE:  // buffer finished playing
			//printf("done one frame\n");
			point_index++;
			/*if(point_index%1920==0)
				printf("\r\n");
			else
			{
				if(point_index%32==0)
				{
					//int c=14;// 1-smile, 14-music, 15-sun
					//printf("%c",c);
					{
						static int icnt=0;
						switch(icnt)
						{
						case 0:
							printf("%c",'|');
							icnt++;
							break;
						case 1:
							printf("%c",'_');
							icnt++;
							break;
						case 2:
							printf("%c",'|');
							icnt++;
							break;
						case 3:
							printf("%c",'-');
							icnt=0;
							break;
						}
					}
				}
			}*/
			{
				if(isfinished==TRUE)
					break;
				static int icnt=0;
				switch(icnt)
				{
				case 0:
					printf("%c\r",'|');
					icnt++;
					break;
				case 1:
					printf("%c\r",'/');
					icnt++;
					break;
				case 2:
					printf("%c\r",'-');
					icnt++;
					break;
				case 3:
					printf("%c\r",'\\');
					icnt=0;
					break;
				}
			}
			{
				PWAVEHDR pWaveHeader = (PWAVEHDR)dwParam1;//系统自动识别是哪一个WAVEHDR播放完毕
				//waveOutUnprepareHeader(hWaveOut, pWaveHeader, sizeof(WAVEHDR) );//播放完后须调用此函数

				/*put 2 frame data together,then send to soundcard to play;otherwise it's too frequency to switch between pWaveHdr1 and pWaveHdr2;
				the audio play quality is badly.*/
	       		if (frame_read(&fr))
				{
					//ad_init();//every frame use the flexible waveformat
					if(fr.protection)	// <---- ... Added 13 April. 2008
						bi.ptr += 2;
					
					if(fr.sync == 1)
					{
						yyj_do_layer3(&fr,pframe1);
						memcpy((unsigned char *)pWaveHeader->lpData,pframe1,g_pcm_point);		
					}
				}
				else
				{
					#if GSVA_PROTECT
					EnterCriticalSection(&CriticalSection);
					#endif
					Sleep(50);
					isfinished=TRUE;
					#if GSVA_PROTECT
					LeaveCriticalSection(&CriticalSection);
					#endif
					return;
				}

				if (frame_read(&fr))
				{
					if(fr.protection)	// <---- ... Added 13 April. 2008
						bi.ptr += 2;
					if(fr.sync == 1)
					{
						yyj_do_layer3(&fr,pframe2);
						memcpy((unsigned char *)pWaveHeader->lpData+g_pcm_point,pframe2,g_pcm_point);
						pWaveHeader->dwBufferLength    = g_pcm_point*2;   
						waveOutWrite(hWaveOut, pWaveHeader, sizeof(WAVEHDR));
					}
				}
				else
				{
					pWaveHeader->dwBufferLength    = g_pcm_point;   
					waveOutWrite(hWaveOut, pWaveHeader, sizeof(WAVEHDR));		
					#if GSVA_PROTECT
					EnterCriticalSection(&CriticalSection);
					#endif
					Sleep(50);
					isfinished=TRUE;
					#if GSVA_PROTECT
					LeaveCriticalSection(&CriticalSection);
					#endif
					return;
				}
			}
			break;
		case WOM_CLOSE:  // connection closed
			//AfxMessageBox("closed");
			break;
	}
} 
/*
void audio_buf_malloc()
{
	pWaveHdr1 = (PWAVEHDR)malloc(sizeof(WAVEHDR));
	pWaveHdr2 = (PWAVEHDR)malloc(sizeof(WAVEHDR));

	pdata1 = (PBYTE)malloc(ONCE_RD_DATA_LENGTH); 
	pdata2 = (PBYTE)malloc(ONCE_RD_DATA_LENGTH); 

	pframe1 = (PBYTE)malloc(ONCE_RD_DATA_LENGTH/2); 
	pframe2 = (PBYTE)malloc(ONCE_RD_DATA_LENGTH/2); 

	if(!pWaveHdr1||!pWaveHdr2||!pdata1||!pdata2||!pframe1||!pframe2)
	{
		printf("can not malloc mem in func audio_buf_malloc().\n");
		return;
	}
}*/
void ad_init()
{
	pWaveHdr1 = (PWAVEHDR)malloc(sizeof(WAVEHDR));
	pWaveHdr2 = (PWAVEHDR)malloc(sizeof(WAVEHDR));

	pdata1 = (PBYTE)malloc(ONCE_RD_DATA_LENGTH); 
	pdata2 = (PBYTE)malloc(ONCE_RD_DATA_LENGTH); 

	pframe1 = (PBYTE)malloc(ONCE_RD_DATA_LENGTH/2); 
	pframe2 = (PBYTE)malloc(ONCE_RD_DATA_LENGTH/2); 

	if(!pWaveHdr1||!pWaveHdr2||!pdata1||!pdata2||!pframe1||!pframe2)
	{
		printf("can not malloc mem in func audio_buf_malloc().\n");
		return;
	}
	waveformat.wFormatTag        = WAVE_FORMAT_PCM;
	waveformat.nChannels         = 2;//fr.stereo;
	waveformat.nSamplesPerSec    = SAMPLE_RATE;//freqs[fr.sampling];
	waveformat.nAvgBytesPerSec   = SAMPLE_RATE*4;//freqs[fr.sampling]*4;//play fast or slow control
	waveformat.nBlockAlign       = 4;
	waveformat.wBitsPerSample    = 16;//one channel=8;two channel=16
	waveformat.cbSize            = 0;

	if(waveOutOpen(&hWaveOut,WAVE_MAPPER,&waveformat,(DWORD)mp3WaveOutProc,0,CALLBACK_FUNCTION)!=MMSYSERR_NOERROR)   
	{   
		free(pWaveHdr1);		free(pWaveHdr2);
		free(pdata1);			free(pdata2);
		free(pframe1);			free(pframe2);		
		pWaveHdr1=NULL;		pWaveHdr2=NULL;
		hWaveOut = NULL;
		printf("open audio device fail\r\n");
		return;
	}
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


void playframe()
{
	//ad_init();//for flexible waveformat setting, modified 13 April. 2008

	/*put 2 frame data together,then send to soundcard to play;otherwise it's too frequency to switch between pWaveHdr1 and pWaveHdr2;
	the audio play quality is badly.*/

	yyj_init_layer3();//just init all global variables, call it one time enough.
	
	if(frame_read(&fr))
	{
		//ad_init();//every song use the same fixed waveformat
		if(fr.protection)	// <---- ... Added 13 April. 2008
			bi.ptr += 2;

		if(fr.sync == 1)
		{
			yyj_do_layer3(&fr,pframe1);
			memcpy(pdata1,pframe1,g_pcm_point);
			//pWaveHdr1->dwBufferLength    = g_pcm_point;
			//waveOutWrite(hWaveOut, pWaveHdr1, sizeof(WAVEHDR));  
		}
	}
	if(frame_read(&fr))
	{
		if(fr.protection)	// <---- ... Added 13 April. 2008
			bi.ptr += 2;

		if(fr.sync == 1)
		{
			yyj_do_layer3(&fr,pframe2);
			memcpy(pdata1+g_pcm_point,pframe2,g_pcm_point);
			pWaveHdr1->dwBufferLength    = g_pcm_point*2;
			waveOutWrite(hWaveOut, pWaveHdr1, sizeof(WAVEHDR));		
		}
	}

	/*put 2 frame data together,then send to soundcard to play;otherwise it's too frequency to switch between pWaveHdr1 and pWaveHdr2;
	the audio play quality is badly.*/
	if(frame_read(&fr))
	{
		if(fr.protection)	// <---- ... Added 13 April. 2008
			bi.ptr += 2;

		if(fr.sync == 1)
		{
			yyj_do_layer3(&fr,pframe1);
			memcpy(pdata2,pframe1,g_pcm_point);
		}
	}
	if(frame_read(&fr))
	{
		if(fr.protection)	// <---- ... Added 13 April. 2008
			bi.ptr += 2;

		if(fr.sync == 1)
		{
			yyj_do_layer3(&fr,pframe2);
			memcpy(pdata2+g_pcm_point,pframe2,g_pcm_point);
			pWaveHdr2->dwBufferLength    = g_pcm_point*2;
			waveOutWrite(hWaveOut, pWaveHdr2, sizeof(WAVEHDR));		
		}
	}
}

void keydeal()
{
	//while(!isexit)
	while(1)
	{
	setbuf(stdin,NULL);
	char ch=getch();
	printf("%c",ch);
	switch(ch)
	{
		case 'q':
			exit(0);
		case 'n'://next song
			#if GSVA_PROTECT
			EnterCriticalSection(&CriticalSection);
			#endif
			isfinished=TRUE;
			#if GSVA_PROTECT
			LeaveCriticalSection(&CriticalSection);
			#endif

			/*waveOutUnprepareHeader(hWaveOut, pWaveHdr1, sizeof(WAVEHDR) );
			waveOutUnprepareHeader(hWaveOut, pWaveHdr2, sizeof(WAVEHDR) );
			waveOutClose(hWaveOut);
			if(waveOutOpen(&hWaveOut,WAVE_MAPPER,&waveformat,(DWORD)mp3WaveOutProc,0,CALLBACK_FUNCTION)!=MMSYSERR_NOERROR)   
			{   
				/*free(pWaveHdr1);
				free(pWaveHdr2);
				pWaveHdr1=NULL;
				pWaveHdr2=NULL;//
				hWaveOut = NULL;
				break;
			}
			waveOutPrepareHeader(hWaveOut,pWaveHdr1,sizeof(WAVEHDR));
			waveOutPrepareHeader(hWaveOut,pWaveHdr2,sizeof(WAVEHDR));*/
			break;
		case 'p'://previous song
			#if GSVA_PROTECT
			EnterCriticalSection(&CriticalSection);
			#endif
			isfinished=TRUE;
			mp3index--;
			mp3index--;
			#if GSVA_PROTECT
			LeaveCriticalSection(&CriticalSection);
			#endif

			/*waveOutUnprepareHeader(hWaveOut, pWaveHdr1, sizeof(WAVEHDR) );
			waveOutUnprepareHeader(hWaveOut, pWaveHdr2, sizeof(WAVEHDR) );
			waveOutClose(hWaveOut);
			if(waveOutOpen(&hWaveOut,WAVE_MAPPER,&waveformat,(DWORD)mp3WaveOutProc,0,CALLBACK_FUNCTION)!=MMSYSERR_NOERROR)   
			{   
				/*free(pWaveHdr1);
				free(pWaveHdr2);
				pWaveHdr1=NULL;
				pWaveHdr2=NULL;//
				hWaveOut = NULL;
				break;
			}
			waveOutPrepareHeader(hWaveOut,pWaveHdr1,sizeof(WAVEHDR));
			waveOutPrepareHeader(hWaveOut,pWaveHdr2,sizeof(WAVEHDR));*/
			break;
		case 'l'://loop
			isloop=!isloop;
			break;
		case 'i'://show info
			isloop?printf("\r\nnow play is loop!\r\n"):printf("now play is not loop!\r\n");
			printf("now play %d/%d.\r\n",(mp3index+wavindex+1),(mp3fileNum+wavfileNum));
			break;
		case 'h'://help
			printf("\b\b\b\b\r");
			printf("/***************************************************************\r\n");
			printf("Mp3 console player. Version 1.0.\r\n");
			printf("usage: 1.put this exe in any directory, then click it, \r\n");
			printf("         it can play all mp3 in the directory.\r\n");
			printf("       2.open cmd, then input \"mp.exe dir\"\r\n");
			printf("Press \'q\' to exit the player.\r\n");
			printf("Press \'p\' to player previous song.\r\n");
			printf("Press \'n\' to play next song.\r\n");
			printf("Press \'h\' to show this help info.\r\n");
			printf("/***************************************************************\r\n");
			break;
		case '\n':
			break;
		default:
			//printf("Can not recognize the command \"%c\".\r\n",ch);
			break;
	}
	}
	//_endthreadex(0);
}

VOID  CALLBACK  OnTimerProc(HWND   hwnd,UINT   uMsg,UINT_PTR   idEvent,DWORD   dwTime)  
{  
	printf("aaa.\r\n");
}   
//回调过程（时钟到来，回调函数被系统自动调用） 
void CALLBACK TimeProc(UINT uID,UINT uMsg,DWORD dwUser,DWORD dw1,DWORD dw2) 
{ 
	char ch=getch();
	printf("%c",'|');
	printf("%c\r",ch);
	switch(ch)
	{
		case 'q':
			exit(0);
		case 'n'://next song
			#if GSVA_PROTECT
			EnterCriticalSection(&CriticalSection);
			#endif
			isfinished=TRUE;
			#if GSVA_PROTECT
			LeaveCriticalSection(&CriticalSection);
			#endif
			break;
		case 'p'://previous song
			#if GSVA_PROTECT
			EnterCriticalSection(&CriticalSection);
			#endif
			isfinished=TRUE;
			mp3index--;
			mp3index--;
			#if GSVA_PROTECT
			LeaveCriticalSection(&CriticalSection);
			#endif
			break;
		case 'l'://loop
			isloop=!isloop;
			break;
		case 'i'://show info
			isloop?printf("\r\nnow play is loop!\r\n"):printf("now play is not loop!\r\n");
			printf("now play %d/%d.\r\n",(mp3index+wavindex+1),(mp3fileNum+wavfileNum));
			break;
		case 'h'://help
			printf("\b\b\b\b\r");
			printf("/***************************************************************\r\n");
			printf("Mp3 console player. Version 1.0.\r\n");
			printf("usage: 1.put this exe in any directory, then click it, \r\n");
			printf("         it can play all mp3 in the directory.\r\n");
			printf("       2.open cmd, then input \"mp.exe dir\"\r\n");
			printf("Press \'q\' to exit the player.\r\n");
			printf("Press \'p\' to player previous song.\r\n");
			printf("Press \'n\' to play next song.\r\n");
			printf("Press \'h\' to show this help info.\r\n");
			printf("/***************************************************************\r\n");
			break;
		case '\n':
			break;
		default:
			//printf("Can not recognize the command \"%c\".\r\n",ch);
			break;
	}
} 
void main(int argc,char *argv[])   
{
	printf("MP3 Player in the console.\r\n");
	if(argc==1)
	{
		ReadAllFiles(".");
	}
	else
	{
		ReadAllFiles(argv[1]);
	}

	if(mp3fileNum==0&&wavfileNum==0)
	{
		printf("no mp3/wav file in current dir and sub-dir.\r\n");
		printf("Press any key to quit......\r\n");
		getch();
		return;
	}

	//rd->filept=NULL;
	#if GSVA_PROTECT
	InitializeCriticalSection(&CriticalSection);
	#endif
	//audio_buf_malloc();
	ad_init();//all songs use the same fixed waveformat
	setbuf(stdin,NULL);
	//SetTimer(NULL,0,1000,OnTimerProc);

#if MULTI_THREAD
	unsigned int id2;
	HANDLE hThread   =   (HANDLE)_beginthreadex(NULL,20000,(unsigned(__stdcall*)(void*))keydeal,NULL, 0, &id2);//multi-thread play
#else
	//启动计时器 
	MMRESULT nIDTimerEvent = timeSetEvent(1000,  0,  TimeProc,  0,  (UINT)TIME_PERIODIC); 
	if( nIDTimerEvent == 0 ) 
		printf("start timer error\r\n");
#endif
loop:
#if MP3_SUPPORT
	for(mp3index=0;mp3index<mp3fileNum;mp3index++)
	{
		/*reset all variable after every song play finished*/
		#if GSVA_PROTECT
		EnterCriticalSection(&CriticalSection);
		#endif
		isfinished=FALSE;
		point_index=0;
		issupported=TRUE;
		#if GSVA_PROTECT
		LeaveCriticalSection(&CriticalSection);
		#endif

		printf("\r\n");
		if(file_open(mp3files[mp3index]) ==NULL)	
		{
			printf("file \" %s \"open fail\r\n",mp3files[mp3index]);
			printf("Press any key to quit!\r\n");
			getch();
			return ;
		}

		parseID3V1(rd->filept);
		skip_id3v2(rd);
#if MULTI_THREAD
{
		unsigned int id;
		_beginthreadex(NULL,20000,(unsigned(__stdcall*)(void*))playframe,NULL, 0, &id);//multi-thread play
}
#else
		playframe();//single-thread play
#endif
		printf("%s is playing\r\n",mp3files[mp3index]);

		/*while(!isfinished)
		{
			Sleep(60);
		}*/

		while(1)
		{
			#if GSVA_PROTECT
			EnterCriticalSection(&CriticalSection);
			#endif
			if(isfinished)
			{
				#if GSVA_PROTECT
				LeaveCriticalSection(&CriticalSection);
				#endif
				break;
			}
			else
			{
				#if GSVA_PROTECT
				LeaveCriticalSection(&CriticalSection);
				#endif
				Sleep(60);
			}	
		}

		if(wavfileNum==0)
		{
			if(isloop)
			{
				if(mp3index==mp3fileNum-1)
				{
					mp3index=-1;
					goto loop;
				}
			}
		}

		if(!issupported)
		{
			printf("can not play this mp3 yet!\n");
			//printf("Currently the player can only play 128kbps bitrate.\n");
		}
		else
		{		
			printf("\r\n%s play finished!\r\n",mp3files[mp3index]);
			printf("total time:%d seconds.\r\n",point_index*52/1000);
		}
		//freerscs();
	}
#endif
	{
		for(int i=0;i<50000;i++)
			for(int j=0;j<10000;j++)
				;//wait for play finished the data in the audio buffer, then isfinished can really be false
	}

#if WAV_SUPPORT
	for(wavindex=0;wavindex<wavfileNum;wavindex++)
	{
		pdata1=(u8 *)malloc(ONCE_RD_DATA_LENGTH);
		pWaveHdr1 = (PWAVEHDR)malloc(sizeof(WAVEHDR));
		pdata2=(u8 *)malloc(ONCE_RD_DATA_LENGTH);
		pWaveHdr2 = (PWAVEHDR)malloc(sizeof(WAVEHDR));
	
		/*reset all variable after every song play finished*/
		isfinished=FALSE;
		point_index=0;		

		printf("%s is playing\r\n",wavfiles[wavindex]);

		parseWavHeader(wavfiles[wavindex]);


#if MULTI_THREAD
{
		unsigned int id;
		_beginthreadex(NULL,20000,(unsigned(__stdcall*)(void*))sound_play, NULL, 0, &id);
}
#else
		sound_play();
#endif
		while(!isfinished)
		{
			Sleep(60);
		}
		printf("\r\n%s play finished!\r\n",wavfiles[wavindex]);
		printf("total time:%d seconds.\r\n",point_index*52/1000);
		//freerscs();

		if(isloop)
		{
			if(wavindex==wavindex-1)
			{
				wavindex=0;
				mp3index=0;
				goto loop;
			}
		}
	}
#endif
waveOutClose(hWaveOut);
	printf("All songs play finished!\r\n");
	printf("Press \'q\' key to quit!\r\n");
	//_endthreadex
	//TerminateThread(hThread,0);
	getch();
	isexit=TRUE;
	freerscs();

	#if GSVA_PROTECT
	DeleteCriticalSection(&CriticalSection);
	#endif
}   

void freerscs()
{
	waveOutUnprepareHeader(hWaveOut, pWaveHdr1, sizeof(WAVEHDR) );
	waveOutUnprepareHeader(hWaveOut, pWaveHdr2, sizeof(WAVEHDR) );
	if (hWaveOut!=NULL)
	{
		waveOutClose(hWaveOut);
		hWaveOut=NULL;
	}	
	if (pdata1!=NULL)
	{
		free(pdata1);
		pdata1=NULL;
	}
	if (pdata2!=NULL)
	{
		free(pdata2);
		pdata2=NULL;
	}
	if (pframe1!=NULL)
	{
		free(pframe1);
		pframe1=NULL;
	}
	if (pframe2!=NULL)
	{
		free(pframe2);
		pframe2=NULL;
	}
	if (pWaveHdr1!=NULL)
	{
		free(pWaveHdr1);
		pWaveHdr1=NULL;
	}
	if (pWaveHdr2!=NULL)
	{
		free(pWaveHdr2);
		pWaveHdr2=NULL;
	}
	
	/*if (rd->filept !=NULL)
	{
	//	fclose(rd->filept );
	}*/
}

void ReadAllFiles(char *lpszDir)
{
	char currentDir[MAX_PATH];
	WIN32_FIND_DATA Win32_Find_Datas;
	HANDLE hFile;
	char tmp[MAX_PATH];
	memset(tmp,0,MAX_PATH);

	if(!SetCurrentDirectory(lpszDir))
	{
		printf("SetCurrentDirectory error:%d\n",GetLastError());
		getch();
		//getchar();//need "enter" key to get the input
		return;
	}

	hFile = FindFirstFile("*.*",&Win32_Find_Datas);
	if(hFile == INVALID_HANDLE_VALUE)
	{ 
		printf("can not findfirstfile\r\n");
		return;
	}

	if(Win32_Find_Datas.cFileName[0] != '.')
	{
		GetCurrentDirectory(MAX_PATH,currentDir);

		//if (strstr(Win32_Find_Datas.cFileName,".mp3"))
		if(strncmp(strlwr(strrev(strdup(Win32_Find_Datas.cFileName))),"3pm.",4)==0)
		{
			sprintf(tmp,"%s\0",Win32_Find_Datas.cFileName);
			strcpy(mp3files[mp3fileNum],currentDir);
			strcat(mp3files[mp3fileNum],"\\");
			strcat(mp3files[mp3fileNum],tmp);
			printf("%s\r\n",mp3files[mp3fileNum]);
			mp3fileNum++;
		}
#if WAV_SUPPORT
		if(strncmp(strlwr(strrev(strdup(Win32_Find_Datas.cFileName))),"vaw.",4)==0)
		{
			sprintf(tmp,"%s\0",Win32_Find_Datas.cFileName);
			strcpy(wavfiles[wavfileNum],currentDir);
			strcat(wavfiles[wavfileNum],"\\");
			strcat(wavfiles[wavfileNum],tmp);
			printf("%s\r\n",wavfiles[wavfileNum]);
			wavfileNum++;
		}
#endif
		if(Win32_Find_Datas.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
		{
			ReadAllFiles(Win32_Find_Datas.cFileName);
		}
	}

	while(FindNextFile(hFile,&Win32_Find_Datas))
	{
		if(Win32_Find_Datas.cFileName [0] == '.')              
			continue;

		GetCurrentDirectory(MAX_PATH,currentDir);

		//if (strstr(Win32_Find_Datas.cFileName,".mp3"))
		if(strncmp(strlwr(strrev(strdup(Win32_Find_Datas.cFileName))),"3pm.",4)==0)
		{
			sprintf(tmp,"%s\0",Win32_Find_Datas.cFileName);
			strcpy(mp3files[mp3fileNum],currentDir);
			strcat(mp3files[mp3fileNum],"\\");
			strcat(mp3files[mp3fileNum],tmp);
			printf("%s\r\n",mp3files[mp3fileNum]);
			mp3fileNum++;
		}
#if WAV_SUPPORT
		if(strncmp(strlwr(strrev(strdup(Win32_Find_Datas.cFileName))),"vaw.",4)==0)
		{
			sprintf(tmp,"%s\0",Win32_Find_Datas.cFileName);
			strcpy(wavfiles[wavfileNum],currentDir);
			strcat(wavfiles[wavfileNum],"\\");
			strcat(wavfiles[wavfileNum],tmp);
			printf("%s\r\n",wavfiles[wavfileNum]);
			wavfileNum++;
		}
#endif
		if(Win32_Find_Datas.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
		{
			ReadAllFiles(Win32_Find_Datas.cFileName);
		}
	}

	SetCurrentDirectory("..");
}

void parseID3V1(FILE *fp)
{
	ID3V1 id3v1;
	fseek(fp,-128,SEEK_END);
	fread(id3v1.Header,1,3,fp);
	if (id3v1.Header[0]!='T'||id3v1.Header[1]!='A'||id3v1.Header[2]!='G')
	{
		printf("no ID3V1 TAG!\r\n");
		return;
	}
	fread(id3v1.Title,1,30,fp);
	printf("Title:%s\r\n",id3v1.Title);

	fread(id3v1.Artist,1,30,fp);
	printf("Artist:%s\r\n",id3v1.Artist);

	fread(id3v1.Album,1,30,fp);
	printf("Album:%s\r\n",id3v1.Album);

	fread(id3v1.Year,1,4,fp);
	printf("Year:%s\r\n",id3v1.Year);

	fread(id3v1.Comment,1,28,fp);
	fread(&id3v1.reserve,1,1,fp);
	fread(&id3v1.track,1,1,fp);
	fread(&id3v1.Genre,1,1,fp);
}
