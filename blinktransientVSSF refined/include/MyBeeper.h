#pragma once

#include <eye/protocol.hpp>
#include <eye/signal.hpp>
#include <pthread.h>

class MyBeeper{

public:
    MyBeeper();
	MyBeeper(basic::time::Timer*);
	~MyBeeper();

	void Play(int freq, int dur);	// freq: frequency in Hz;	dur: duration in ms
	float GetBeepTime();

// private:
	pthread_mutex_t _mut;
	pthread_cond_t _cond;

	pthread_t _threadID;

	bool _isPaused;
	bool _isFinished;
	int _freq;	// Hz
	int _dur;	// ms
	basic::time::Timer *_pTimer;		// pointer to an external timer
	std::chrono::milliseconds _tBeep;
};