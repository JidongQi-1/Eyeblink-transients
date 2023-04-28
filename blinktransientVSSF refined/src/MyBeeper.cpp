#include <stdio.h>
#include <iostream>
#include <string>
#include "MyBeeper.h"

#define LINUX

using namespace std;

#ifdef WINDOWS
#include <Window.h>
#include <io.h>
void BEEP(int freq, int dur)
{
	Beep(freq, dur);
}

#elif defined(LINUX)
#include <stdio.h>
// #include<ncurses.h>
// #include <sys/ioctl.h>
// #include <fcntl.h>
// #include <linux/kd.h>
void BEEP(int freq, int dur)
{
	system( (string("beep -f ") + to_string(freq) + " -l " + to_string(dur) + " -r 1").c_str() );
	// ioctl( fd, KDMKTONE, (ms << 16 | 1193180 / freq) );
	// beep();
	// int fd = open("/dev/console", O_WRONLY);
 	// if(fd != -1) ioctl( fd, KDMKTONE, dur<<16 | 1193180/freq );
}

#else
#include <stdio.h>
void BEEP(int freq, int dur)
{
	cout << "\a" << flush;
}
#endif


void* ThreadFunc(void *pParams)
{
	MyBeeper* This = (MyBeeper*) pParams;
	while(!This->_isFinished){
        pthread_mutex_lock(&(This->_mut));
		while(This->_isPaused)
		    pthread_cond_wait(&(This->_cond), &(This->_mut));
        pthread_mutex_unlock(&(This->_mut));

		if(This->_pTimer) This->_tBeep = This->_pTimer->getTime();
		BEEP(This->_freq,This->_dur);
		This->_isPaused = true;
	}
	return nullptr;
}


MyBeeper::MyBeeper()
{
    _pTimer = nullptr;
    _isPaused = true;
    _isFinished = false;
    _freq = 3000;
    _dur = 100;
    _threadID = 0;
}

MyBeeper::MyBeeper(basic::time::Timer* p)
{
	_pTimer = p;
	_isPaused = true;
	_isFinished = false;
    _freq = 3000;
    _dur = 100;

	_mut = PTHREAD_MUTEX_INITIALIZER;
	_cond = PTHREAD_COND_INITIALIZER;
	if(pthread_mutex_init(&_mut, NULL) != 0) printf("MyBeeper: mutex init error\n");
	if(pthread_cond_init(&_cond, NULL) != 0) printf("MyBeeper: cond init error\n");

	int err;
	if((err = pthread_create(&_threadID, NULL, ThreadFunc, (void*)this)) != 0) printf("MyBeeper: can't create thread: %s\n", strerror(err));
}

MyBeeper::~MyBeeper()
{
	_isFinished = true;
	if(_threadID) pthread_cancel(reinterpret_cast<pthread_t>(&_threadID));
	//if(0 == pthread_join(reinterpret_cast<pthread_t>(&_threadID), NULL)) printf("MyBeeper: thread is over\n");
}

void MyBeeper::Play(int freq = 3000, int dur = 100)
{
	_freq = freq;
	_dur = dur;

	pthread_mutex_lock(&_mut);
	_isPaused = false;
    pthread_cond_signal(&_cond);
    pthread_mutex_unlock(&_mut);
}

float MyBeeper::GetBeepTime()
{
	return _tBeep.count();	// ms
}