/*  Copyright 2010 Yoshihiro

    This file is part of Desmume360 for xbox360.

    Desmume360 is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Desmume60 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Desmume360; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/
#include <iostream>
#include <stdlib.h>
#include "mc.h"
#include "gfx3d.h"
#include "types.h"
#include "MMU.h"
#include "NDSSystem.h"
#include "firmware.h"
#include "debug.h"
#include "render3D.h"
#include "rasterize.h"
#include "saves.h"
#include <android/log.h>
#include "gfx3d.h"
#include <pthread.h>
#include <jni.h>
#include <string.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include "sndsdl.h"
#ifdef HAVE_DYNAREC
#include "dynarec/cpu.h"
#endif
#include "android/log.h"

#define LOGE(...) __android_log_print(ANDROID_LOG_INFO,   "JNI_DEBUGGING", __VA_ARGS__)

char ExtFPS[1000];
pthread_mutex_t DsExec;

volatile bool execute = FALSE;

static float nds_screen_size_ratio = 2.0f;

SoundInterface_struct *SNDCoreList[] = {
  &SNDSDL,
  &SNDDummy,
  &SNDDummy,
  NULL
};

GPU3DInterface *core3DList[] = {
&gpu3DNull,
&gpu3DRasterize,
NULL
};
static unsigned short keypad = 0;
u32 last_cycle = 0;
char statepath[256];
int romloaded = 0;
volatile int paused = 0;

static int FrameLimit = 1;
bool frameAdvance = false;
int autoframeskipenab=1;
int frameskiprate=0;
int emu_paused = 0;

//--------------------------------------------------------------------------------------
// Color values
const u32        INFO_COLOR = 0xffffff00;          // Yellow


#define IN_GAME						0
#define IN_GAME_PAUSED				1
#define MAIN_MENU					2
#define INIT_SYSTEM					3
#define INIT_MENU_SYSTEM		    4

class Sample
{
    u32 m_dwCurrentImageIndex;

	typedef struct _filenamestruct {
		char name[2000] ;
		unsigned char filename[2000] ;
	} FILENAME ;

	u32 topIdx;
	u32 curr;
	int spos;

	unsigned long numfiles;
	int		m_nXOffset, m_nFontHeight ;
	int		m_namesPerPage ;
	int		m_state;

	int		Skins;
	u32 theWidth;
	u32 theHeight;


public:
	bool       DSInit();
	void			ExecEmu();
	bool			InitializeWithScreen();
	void			MoveCursor();
	void			QuickSort( int lo, int hi );
	void			FindAvailRoms();
	void			DSPAD();
	virtual bool Initialize();
private:

    virtual bool Update();
};

Sample atgApp;

//struct mouse_status mouse;
int StopThread;


struct my_config {
  int disable_sound;

  int disable_limiter;

  int engine_3d;

  int frameskip;

  int firmware_language;

  int savetype;

  const char *nds_file;
  const char *cflash_disk_image_file;
};


const char * save_type_names[] = {
    "Autodetect",
    "EEPROM 4kbit",
    "EEPROM 64kbit",
    "EEPROM 512kbit",
    "FRAM 256kbit",
    "FLASH 2mbit",
    "FLASH 4mbit",
    NULL
};

static void
init_config( struct my_config *config) {
  config->disable_limiter = 0;

  config->engine_3d = 1;

  config->nds_file = NULL;

  config->frameskip = 0;

  config->cflash_disk_image_file = NULL;

  config->savetype = 0;

  /* use the default language */
  config->firmware_language = -1;

  config->disable_sound=10;
  /*mouse.xx = 0;
  mouse.yy = 0;
  mouse.x  = 0;
  mouse.y  = 0;*/
}


int savetype=MC_TYPE_AUTODETECT;
u32 savesize=1;

//--------------------------------------------------------------------------------------
// Name: main()
// Desc: Entry point to the program
//--------------------------------------------------------------------------------------
char rompath[256];
static volatile bool doterminate;
static volatile bool terminated;
pthread_t mainThread;

extern "C"
{
	JavaVM* gJVM;
	JNIEnv* javaEnv;
	_jclass* FilePathRef;

	JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
	{
		gJVM = vm;
		return JNI_VERSION_1_6;

	}

	void *initialize_thread(void * id)
	{
		LOGE("thread in");
		int res = gJVM->AttachCurrentThread(&javaEnv, NULL);
		atgApp.Initialize();
		LOGE("exiting thread");
		gJVM->DetachCurrentThread();
		//terminated = true;
		//pthread_exit(NULL);
	}

	JNIEXPORT jdouble JNICALL Java_com_qingping_EmulatorActivity_entry(JNIEnv* envi, jobject jobj, jstring path)
	{
		const char* path_ptr = envi->GetStringUTFChars(path, NULL);
		strcpy(rompath, path_ptr);
		LOGE(rompath);
		//javaEnv=envi;
		//global class reference
		jclass clazz = envi->FindClass("com/qingping/FilePath");
		if(clazz==NULL)
		{
			LOGE("Class not found");
		}

		FilePathRef = (_jclass*)envi->NewGlobalRef(clazz);
		envi->DeleteLocalRef(clazz);
		if(FilePathRef == NULL)
			LOGE("Global class reference failed");

		pthread_create(&mainThread,NULL, initialize_thread, NULL);
		LOGE("thread created");
		envi->ReleaseStringUTFChars(path, path_ptr);
		LOGE("string released");
		//pthread_exit(NULL);

		return 0;
	}

	JNIEXPORT void JNICALL Java_com_qingping_EmulatorActivity_deInitNDS(JNIEnv* envi, jobject jobj)
	{
		doterminate=true;
		terminated=true;
	}


}




void Sample::MoveCursor()
{

}

bool useMmapForRomLoading=false;


#define FRAMESKIP 4

#include <time.h>

volatile bool isFinishedInit=false;

void* MainCycle(void* ptr)
{

	int res = gJVM->AttachCurrentThread(&javaEnv, NULL); // res == 0
	timespec render;
	render.tv_sec=0;
	render.tv_nsec=10;
	pthread_mutex_init(&DsExec,NULL);
    // Give this thread a name
	StopThread = 0;

#ifdef HAVE_NEON
	static const unsigned int x = 0x04086060;
		static const unsigned int y = 0x03000000;
		int r;
		asm volatile (
			"fmrx	%0, fpscr			\n\t"	//r0 = FPSCR
			"and	%0, %0, %1			\n\t"	//r0 = r0 & 0x04086060
			"orr	%0, %0, %2			\n\t"	//r0 = r0 | 0x03000000
			"fmxr	fpscr, %0			\n\t"	//FPSCR = r0
			: "=r"(r)
			: "r"(x), "r"(y)
		);
#endif
	/*while(!doterminate)
		usleep(100000);
		*/
	int counter=0;

	struct timespec start, finish;
	double elapsed;

	clock_gettime(CLOCK_MONOTONIC, &start);

	while(!doterminate)
	{
		if(execute)
		{

		    pthread_mutex_lock (&DsExec);

		    NDS_beginProcessingInput();
		    NDS_endProcessingInput();
			NDS_exec<false>();

			pthread_mutex_unlock(&DsExec);



			if(counter == FRAMESKIP)
			{
				clock_gettime(CLOCK_MONOTONIC, &finish);

				elapsed = (finish.tv_sec - start.tv_sec);
				elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

				if(elapsed < ((double)FRAMESKIP)/((double)60.0))
				{
					struct timespec sleep;
					sleep.tv_sec = finish.tv_sec - start.tv_sec;
					sleep.tv_nsec = finish.tv_nsec - start.tv_nsec;

					nanosleep(&sleep, NULL);
				}

				clock_gettime(CLOCK_MONOTONIC, &start);
				counter=0;
			}
			else
			{
				counter++;
				NDS_SkipNextFrame();
			}

			nanosleep(&render, NULL);
		}
	}

	NDS_DeInit();
	isFinishedInit=false;
	LOGE("deinit success");
	gJVM->DetachCurrentThread();
}

extern "C"
{
	pthread_t cycleThread;

	void CreateMainThread()
	{
		isFinishedInit=true;
		pthread_create(&cycleThread,NULL, MainCycle, NULL);
		//MainCycle();
	}

	JNIEXPORT jboolean JNICALL Java_com_qingping_EmulatorActivity_isFinishedInit(JNIEnv* envi, jobject jobj)
	{
			return isFinishedInit;
	}
}



bool Sample::DSInit()
{
  bool hr;
  struct my_config my_config;
  struct NDS_fw_config_data fw_config;

  /* default the firmware settings, they may get changed later */

  NDS_FillDefaultFirmwareConfigData( &fw_config);

  init_config( &my_config);

    /* use any language set on the command line */
  if ( my_config.firmware_language != -1)
  {
    fw_config.language = my_config.firmware_language;
  }

  LOGE("num cores %d", sysconf(_SC_NPROCESSORS_CONF));

  CommonSettings.num_cores=sysconf(_SC_NPROCESSORS_CONF);

  NDS_Init();

  /* Create the dummy firmware */
  NDS_CreateDummyFirmware( &fw_config);

  if ( true) {
    SPU_ChangeSoundCore(SNDCORE_SDL, 735 * 4);
  }

	NDS_3D_ChangeCore(my_config.engine_3d);

  	//printf(rompath+"game:\\roms\\%s",files[curr].filename);

	//printf(statepath+"game:\\state\\%s",files[curr].filename);
	//char *pt = strchr(statepath, '.');
	//if (pt) *pt = 0;
	strcat(statepath, ".dst");

  if (NDS_LoadROM( rompath, my_config.cflash_disk_image_file) < 0) {
    printf("error while loading %s\n", rompath);
  }

   romloaded = TRUE;

   execute = TRUE;

	 //if( FAILED( m_pd3dDevice->CreateTexture(256, 384, 1,0,D3DFMT_LIN_A1B5G5R5, D3DUSAGE_CPU_CACHED_MEMORY, &m_pCurrentImage,0)))
		// return 2;


   	doterminate = false;
	terminated = false;
	CreateMainThread();


	 m_state = IN_GAME;

	return true;
}

extern "C"
{

void NDS_Pause()
{
	if (!paused)
	{
		emu_halt();
		paused = TRUE;
		SPU_Pause(1);
		while (!paused) {}
		LOGE("Emulation paused\n");
	}
}

void NDS_UnPause()
{
	if (romloaded && paused)
	{
		paused = FALSE;
		execute = TRUE;
		SPU_Pause(0);
		LOGE("Emulation unpaused\n");
	}
}

JNIEXPORT void JNICALL Java_com_qingping_EmulatorActivity_emulationPause(JNIEnv* envi, jobject jobj)
{
	NDS_Pause();
}

JNIEXPORT void JNICALL Java_com_qingping_EmulatorActivity_emulationUnPause(JNIEnv* envi, jobject jobj)
{
	NDS_UnPause();
}

}


bool Sample::InitializeWithScreen()
{

	m_nXOffset = 0 ;
	m_nFontHeight = 16 ;


	if ( m_nFontHeight < 1 )
		m_nFontHeight = 16 ;


	m_namesPerPage =  (20*14 ) / ( m_nFontHeight+2) ;

	curr = 0 ;
	topIdx = 0 ;
	numfiles = 0 ;



	m_state = MAIN_MENU ;

	GLuint loc = 0;



   return true;
}

//--------------------------------------------------------------------------------------
// Name: Initialize
// Desc: This creates all device-dependent display objects.
//--------------------------------------------------------------------------------------
bool Sample::Initialize()
{
	DSInit();


	InitializeWithScreen();

	return true;
}

/* Set mouse coordinates */
void set_mouse_coord(signed long x,signed long y)
{

}

extern "C"
{

	JNIEXPORT void JNICALL Java_com_qingping_EmulatorActivity_setButtons(JNIEnv* envi, jobject jobj,
			jboolean left, jboolean right, jboolean up, jboolean down, jboolean start, jboolean select, jboolean a, jboolean b, jboolean x, jboolean y, jboolean l, jboolean r)
	{
		NDS_setPad(right, left, down, up, select, start, b, a, y, x, l, r, false, false);
	}

	JNIEXPORT void JNICALL Java_com_qingping_EmulatorActivity_touch(JNIEnv* envi, jobject jobj, jint x, jint y)
	{
		NDS_setTouchPos((u16)x, (u16)y);
	}

	JNIEXPORT void JNICALL Java_com_qingping_EmulatorActivity_releaseTouch(JNIEnv* envi, jobject jobj)
	{
		NDS_releaseTouch();
	}

	JNIEXPORT void JNICALL Java_com_qingping_EmulatorActivity_saveState(JNIEnv* envi, jobject jobj, jint slotNum)
    {
#ifdef HAVE_DYNAREC
			savestate_slot(0);
			execute=true;
			paused = FALSE;
#else
			savestate_slot(1);
#endif
    }

	JNIEXPORT void JNICALL Java_com_qingping_EmulatorActivity_loadState(JNIEnv* envi, jobject jobj, jint slotNum)
    {
#ifdef HAVE_DYNAREC
			loadstate_slot(0);
			execute=true;
			paused = FALSE;
#else
			loadstate_slot(1);
#endif
    }

//settings

	JNIEXPORT void JNICALL Java_com_qingping_SettingsActivity_enableSound(JNIEnv* envi, jobject jobj, jint slotNum)
	{
		LOGE("enabling sound");
		SNDSDL.UnMuteAudio();
	}

	JNIEXPORT void JNICALL Java_com_qingping_SettingsActivity_disableSound(JNIEnv* envi, jobject jobj, jint slotNum)
	{
		LOGE("disabling sound");
		SNDSDL.MuteAudio();
	}
}

void Sample::DSPAD()
{

}

void Sample::ExecEmu()
{

  /* if(mouse.down) NDS_setTouchPos(mouse.x, mouse.y);
    if(mouse.click)
      {
        NDS_releaseTouch();
        mouse.click = FALSE;
      }

    DSPAD();*/
}



void OnScreenDebugMessage( const char* string, ...)
{

    //va_list pArgList;
    //va_start( pArgList, string );

    //LOGE( ExtFPS+string+pArgList );
    //LOGE( ExtFPS+L"\n" );
	//va_end( pArgList );
}

//--------------------------------------------------------------------------------------
// Name: Update
// Desc: Called once per frame, the call is the entry point for animating
//       the scene.
//--------------------------------------------------------------------------------------
bool Sample::Update()
{

    return true;
}

//--------------------------------------------------------------------------------------
// Name: Render
// Desc: Sets up render states, clears the viewport, and renders the scene.
//--------------------------------------------------------------------------------------
#include <iostream>
#include <sstream>


extern "C"
{
#define TEXWIDTH	256
#define TEXHEIGHT	512
#define WIDTH 256;
#define HEIGHT 384;

u16 colors[256*512];

const float vertices[6*2]={0,0,256,0,256,384,0,0,256,384,0,384};

const float texCoord[6*2]={0.0f,384.0f/512.0f,1.0f  ,384.0f/512.0f, 1.0f,0.0f,0.0f,384.0f/512.0f, 1.0f,0.0f,0.0f,0.0f};

GLuint      texture[1];

#define RATIO (256.0/384.0)


JNIEXPORT jdouble JNICALL Java_com_qingping_ImageRenderer_initGL(JNIEnv *env, jobject jobj, jdouble w, jdouble h)
{

	glClearColor (0.0,0.0,0.0,1.0);

	double r = w/h;
	if(r>RATIO)
		glViewport((w-h/RATIO)/2,0,h/RATIO,h);
	else
		glViewport(0,h-w/RATIO,w,w/RATIO);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	glOrthof(0,256,0,384, -1, 1);

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, texture);

	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXWIDTH,
				TEXHEIGHT, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5,
				colors);
	glTexParameterf(GL_TEXTURE_2D,
	  		GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,
	  		GL_TEXTURE_MAG_FILTER, GL_LINEAR);



	return 0;
}
#define BGR555_TO_RGB565(col) (((col<<1)&0x07E0)|((col&0x001F)<<11)|((col>>10)&0x001F))

JNIEXPORT jdouble JNICALL  Java_com_qingping_ImageRenderer_render()
{
	pthread_mutex_lock(&DsExec);
	u16 *sourceTemp=(u16*)GPU_screen;
	/*u8 *out=(u8*)&GPU_screen;
	char buffer [33];
	for(int i=0;i<4*256*192;i++)
	{
		std::stringstream sstr;
		sstr << (int)out[i];
		std::string str1 = sstr.str();
		LOGE(str1.c_str());
	}*/

	for(int i=0; i < 256*384; i++)
	{
		colors[i]=BGR555_TO_RGB565(sourceTemp[i]);
	}



	pthread_mutex_unlock(&DsExec);

	glClear(GL_COLOR_BUFFER_BIT);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, TEXWIDTH, TEXHEIGHT, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, colors);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer( 2, GL_FLOAT, 0, &vertices );
	glTexCoordPointer( 2, GL_FLOAT, 0, &texCoord);

	glDrawArrays( GL_TRIANGLES, 0, 6);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	return 0;

}
}




