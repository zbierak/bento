/*
 * timer.h
 *
 *  Created on: Nov 22, 2013
 *      Author: zbierak
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>

namespace bento {

#if _WIN32||_WIN64
	#include <windows.h>
	#define TIME_TYPE LARGE_INTEGER 
#else 
 	#include <sys/time.h>
	#define TIME_TYPE struct timeval
#endif 

class Timer
{
private:
	TIME_TYPE startTime;
	TIME_TYPE stopTime;
	
	#if _WIN32||_WIN64
		double frequency;
	#endif
public:
	// start the timer now
	Timer()
	{
		#if _WIN32||_WIN64
			LARGE_INTEGER proc_freq;
			QueryPerformanceFrequency(&proc_freq);
			frequency = (double)proc_freq.QuadPart;
		#endif

		restart();
	}

	// start the timer from one point in time
	Timer(TIME_TYPE _startTime)
	{
		#if _WIN32||_WIN64
			LARGE_INTEGER proc_freq;
			QueryPerformanceFrequency(&proc_freq);
			frequency = (double)proc_freq.QuadPart;
		#endif		

		startTime = _startTime;	
	}

	// restart the timer
	void restart()
	{
		#if _WIN32||_WIN64
			QueryPerformanceCounter(&startTime);
		#else 
			gettimeofday(&startTime, NULL);
		#endif
	}

	// return elapsed time in seconds
	double stop()
	{
		#if _WIN32||_WIN64
			QueryPerformanceCounter(&stopTime);
			return ((stopTime.QuadPart - startTime.QuadPart) / frequency);
		#else 
			gettimeofday(&stopTime, NULL);
			return (stopTime.tv_sec - startTime.tv_sec) + (stopTime.tv_usec - startTime.tv_usec) / 1000000.0;
		#endif
	}


	struct Now
	{
		// return current time in milliseconds
		static uint64_t milliseconds()
		{
			TIME_TYPE timeIsNow;

			#if _WIN32||_WIN64
				LARGE_INTEGER proc_freq;
				QueryPerformanceFrequency(&proc_freq);
				double frequency = ((double)proc_freq.QuadPart) / 1000.0;
				QueryPerformanceCounter(&timeIsNow);
				return static_cast<uint64_t>((stopTime.QuadPart - startTime.QuadPart) / frequency);
			#else
				gettimeofday(&timeIsNow, NULL);
				return static_cast<uint64_t>(1000 * static_cast<uint64_t>(timeIsNow.tv_sec) + timeIsNow.tv_usec / 1000.0);
			#endif
		}

		// return current time in seconds
		static double seconds()
		{
			TIME_TYPE timeIsNow;

			#if _WIN32||_WIN64
				LARGE_INTEGER proc_freq;
				QueryPerformanceFrequency(&proc_freq);
				double frequency = ((double)proc_freq.QuadPart);
				QueryPerformanceCounter(&timeIsNow);
				return (stopTime.QuadPart - startTime.QuadPart) / frequency;
			#else
				gettimeofday(&timeIsNow, NULL);
				return timeIsNow.tv_sec + timeIsNow.tv_usec / 1000000.0;
			#endif
		}
	};

};

}

#endif
