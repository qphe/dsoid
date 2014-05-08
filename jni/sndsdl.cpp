/*
	Copyright (C) 2005-2006 Theo Berkau
	Copyright (C) 2006-2010 DeSmuME team

	This file is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with the this software.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>

//#include <SDL.h>
#include <jni.h>
#include "types.h"
#include "SPU.h"
#include "sndsdl.h"
#include "debug.h"
#include "android/log.h"
#include "pthread.h"


#define LOGE(...) __android_log_print(ANDROID_LOG_INFO,   "JNI_DEBUGGING", __VA_ARGS__)


#ifdef _XBOX
#include <xtl.h>
#include <VectorIntrinsics.h>
#include <process.h>
#endif

int SNDSDLInit(int buffersize);
void SNDSDLDeInit();
void SNDSDLUpdateAudio(s16 *buffer, u32 num_samples);
u32 SNDSDLGetAudioSpace();
void SNDSDLMuteAudio();
void SNDSDLUnMuteAudio();
void SNDSDLSetVolume(int volume);

SoundInterface_struct SNDSDL = {
SNDCORE_SDL,
"SDL Sound Interface",
SNDSDLInit,
SNDSDLDeInit,
SNDSDLUpdateAudio,
SNDSDLGetAudioSpace,
SNDSDLMuteAudio,
SNDSDLUnMuteAudio,
SNDSDLSetVolume
};

static u16 *stereodata16;
static u32 soundoffset;
static volatile u32 soundpos;
static u32 soundlen;
static u32 soundbufsize;
//static SDL_AudioSpec audiofmt;

//////////////////////////////////////////////////////////////////////////////





/*static void MixAudio(void *userdata, Uint8 *stream, int len) {
   int i;
   Uint8 *soundbuf=(Uint8 *)stereodata16;

   for (i = 0; i < len; i++)
   {
      if (soundpos >= soundbufsize)
         soundpos = 0;

      stream[i] = soundbuf[soundpos];
      soundpos++;
   }
}*/

//////////////////////////////////////////////////////////////////////////////
jint i;
jobject audioTrack;
jmethodID constructor;
jmethodID write_audio;
jclass cls;
extern JNIEnv* javaEnv;
pthread_t soundThread;

static volatile bool doterminate;
static volatile bool terminated;

extern JavaVM* gJVM;
short stopBuff[44100];
jshortArray shortBuff;
JNIEnv* soundThreadEnv;

extern volatile bool execute;

void * WINAPI SNDXBOXThread(void *id)
{
	int res = gJVM->AttachCurrentThread(&soundThreadEnv, NULL);
	timespec render;
	render.tv_sec=0;
	render.tv_nsec=10;

	while(!doterminate)
	{
		if(execute)
		{
			SPU_Emulate_user();
			nanosleep(&render, NULL);
		}
	}

	terminated = true;
	gJVM->DetachCurrentThread();

	return 0;
}

int SNDSDLInit(int buffersize)
{
  /* if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
      return -1;

   audiofmt.freq = 44100;
   audiofmt.format = AUDIO_S16SYS;
   audiofmt.channels = 2;
   audiofmt.samples = (audiofmt.freq / 60) * 2;
   audiofmt.callback = MixAudio;
   audiofmt.userdata = NULL;*/

   //samples should be a power of 2 according to SDL-doc
   //so normalize it to the nearest power of 2 here
	LOGE("sound init");


	jobject localShortBuff = javaEnv->NewShortArray(44100);
	shortBuff = static_cast<jshortArray>(javaEnv->NewGlobalRef(localShortBuff));

   u32 normSamples = 512;
   //while (normSamples < audiofmt.samples)
      //normSamples <<= 1;



   jclass localCls = javaEnv->FindClass("android/media/AudioTrack");
   cls = reinterpret_cast<jclass>(javaEnv->NewGlobalRef(localCls));
   if(!cls)
	   LOGE("can't find class");

   jmethodID bufferSizeFunc = javaEnv->GetStaticMethodID(cls, "getMinBufferSize", "(III)I");
  u32 audioTrackBuff = javaEnv->CallStaticIntMethod(cls, bufferSizeFunc, 44100, 12, 2);
  LOGE("buffer size %u", audioTrackBuff);

   constructor = javaEnv->GetMethodID(cls, "<init>", "(IIIIII)V");
   if(!constructor)
   	   LOGE("can't find constructor");

   jobject localAudioTrack = javaEnv->NewObject(cls, constructor, 3, 44100, 12, 2, audioTrackBuff, 1);
   audioTrack = javaEnv->NewGlobalRef(localAudioTrack);
   if(!audioTrack)
      	   LOGE("can't create object");



   write_audio = javaEnv->GetMethodID(cls, "write", "([SII)I");
   if(!write_audio)
         	   LOGE("can't find write function");
   //audiofmt.samples = normSamples;
   
   soundlen = 44100 / 60; // 60 for NTSC
   soundbufsize = buffersize * sizeof(s16) * 2;

  /* if (SDL_OpenAudio(&audiofmt, NULL) != 0)
   {
      return -1;
   }*/

   if ((stereodata16 = (u16 *)malloc(soundbufsize)) == NULL)
      return -1;

   memset(stereodata16, 0, soundbufsize);

   soundpos = 0;




   //SDL_PauseAudio(0);
   jmethodID function = javaEnv->GetMethodID(cls, "play", "()V");
   javaEnv->CallVoidMethod(audioTrack, function);


   	doterminate = false;
	terminated = false;
	pthread_create(&soundThread,NULL, SNDXBOXThread, NULL);
	//CreateThread(0,0,SNDXBOXThread,0,0,0);
	LOGE("sound init finished");

   return 0;
}

//////////////////////////////////////////////////////////////////////////////
void SNDSDLDeInit()
{
	doterminate = true;
	while(!terminated) {
		usleep(1000);
	}

	LOGE("Starting to deinit");

	jmethodID function;

   //SDL_CloseAudio();
  /* function = javaEnv->GetMethodID(cls, "flush", "()V");
   if(!function)
	   LOGE("flush not found");
   javaEnv->CallVoidMethod(audioTrack, function);*/

	javaEnv->SetShortArrayRegion(shortBuff, 0, 44100, (short*)stopBuff);
	s32 writeval = javaEnv->CallIntMethod(audioTrack, write_audio, shortBuff, 0, 44100);
	LOGE("write filler vals %x", writeval);

   function = javaEnv->GetMethodID(cls, "stop", "()V");
   if(!function)
   	   LOGE("stop not found");
   javaEnv->CallVoidMethod(audioTrack, function);

   function = javaEnv->GetMethodID(cls, "release", "()V");
   if(!function)
   	   LOGE("release not found");
   javaEnv->CallVoidMethod(audioTrack, function);

   javaEnv->DeleteGlobalRef(cls);
   javaEnv->DeleteGlobalRef(audioTrack);
   javaEnv->DeleteGlobalRef(shortBuff);

   LOGE("object freed");

   if (stereodata16)
      free(stereodata16);
}

//////////////////////////////////////////////////////////////////////////////
void SNDSDLUpdateAudio(s16 *buffer, u32 num_samples)
{
   u32 copy1size=0, copy2size=0;
   //SDL_LockAudio();

   if ((soundbufsize - soundoffset) < (num_samples * sizeof(s16) * 2))
   {
      copy1size = (soundbufsize - soundoffset);
      copy2size = (num_samples * sizeof(s16) * 2) - copy1size;
   }
   else
   {
      copy1size = (num_samples * sizeof(s16) * 2);
      copy2size = 0;
   }

   //memcpy((((u8 *)stereodata16)+soundoffset), buffer, copy1size);

   //for(int i=0;i<num_samples*2;i++)


   soundThreadEnv->SetShortArrayRegion(shortBuff, 0, num_samples*2, (short*)buffer);

   s32 writeval = soundThreadEnv->CallIntMethod(audioTrack, write_audio, shortBuff, 0, copy1size/2);

//   ScspConvert32uto16s((s32 *)leftchanbuffer, (s32 *)rightchanbuffer, (s16 *)(((u8 *)stereodata16)+soundoffset), copy1size / sizeof(s16) / 2);

   if (copy2size)
   {
	   writeval = soundThreadEnv->CallIntMethod(audioTrack, write_audio, shortBuff, copy1size/2, copy2size/2);
   }
     // memcpy(stereodata16, ((u8 *)buffer)+copy1size, copy2size);
//      ScspConvert32uto16s((s32 *)leftchanbuffer, (s32 *)rightchanbuffer, (s16 *)stereodata16, copy2size / sizeof(s16) / 2);

   soundoffset += copy1size + copy2size;
   soundoffset %= soundbufsize;
  // SDL_UnlockAudio();
}

//////////////////////////////////////////////////////////////////////////////

u32 SNDSDLGetAudioSpace()
{
   u32 freespace=0;

   if (soundoffset > soundpos)
      freespace = soundbufsize - soundoffset + soundpos;
   else
      freespace = soundpos - soundoffset;

   return (44100/ sizeof(s16) / 2);
}

//////////////////////////////////////////////////////////////////////////////

void SNDSDLMuteAudio()
{
	jmethodID function = javaEnv->GetMethodID(cls, "pause", "()V");
	javaEnv->CallVoidMethod(audioTrack, function);
}

//////////////////////////////////////////////////////////////////////////////

void SNDSDLUnMuteAudio()
{
	jmethodID function = javaEnv->GetMethodID(cls, "play", "()V");
	javaEnv->CallVoidMethod(audioTrack, function);
}

//////////////////////////////////////////////////////////////////////////////

void SNDSDLSetVolume(int volume)
{
}

//////////////////////////////////////////////////////////////////////////////
