#ifndef CRITICAL_SECTION_H

#define CRITICAL_SECTION_H

#ifdef __WIN32
#include <Winsock2.h>
#include <Windows.h>
#else
#ifdef __linux__
#include <pthread.h>
#else
#pragma message("Using std::mutex and std::condition_variable")
#include <mutex>
#include <condition_variable>
#endif
#endif
#include <condition_variable>
#include <numeric>

class ConditionVariable;

class CriticalSection
{
public:
	CriticalSection()
	{
#ifdef __WIN32
		InitializeCriticalSection(&cs);
#else
#ifdef __linux__
		pthread_mutex_init(&mtx, NULL);
#else

#endif
#endif
	}
	~CriticalSection()
	{
#ifdef __WIN32
		DeleteCriticalSection(&cs);
#else
#ifdef __linux__
		pthread_mutex_destroy(&mtx);
#else

#endif
#endif
	}
	void lock()
	{
#ifdef __WIN32
		EnterCriticalSection(&cs);
#else
#ifdef __linux__
		pthread_mutex_lock(&mtx);
#else
		mtx.lock();
#endif
#endif
	}
	void unlock()
	{
#ifdef __WIN32
		LeaveCriticalSection(&cs);
#else
#ifdef __linux__
		pthread_mutex_unlock(&mtx);
#else
		mtx.unlock();
#endif
#endif
	}
	bool try_lock()
	{
#ifdef __WIN32
		return TryEnterCriticalSection(&cs);
#else
#ifdef __linux__
		return pthread_mutex_trylock(&mtx);
#else
		return mtx.try_lock();
#endif
#endif
	}

private:
#ifdef __WIN32
	CRITICAL_SECTION cs;
#else
#ifdef __linux__
	pthread_mutex_t mtx;
#else
	std::mutex mtx;
#endif
#endif
	friend class ConditionVariable;
};

class ConditionVariable
{
public:
	ConditionVariable()
	{
#ifdef __WIN32
		InitializeConditionVariable(&cv);
#else
#ifdef __linux__
		pthread_cond_init(&cv, NULL);
#else
		
#endif
#endif
	}
	~ConditionVariable()
	{
#ifdef __WIN32

#else
#ifdef __linux__
		pthread_cond_destroy(&cv);
#else

#endif
#endif
	}
	void wait(CriticalSection &cs)
	{
#ifdef __WIN32
		SleepConditionVariableCS(&cv, &cs.cs, std::numeric_limits<DWORD>::infinity());
#else
#ifdef __linux__
		pthread_cond_wait(&cv, &cs.mtx);
#else
		std::unique_lock<std::mutex> lck(cs.mtx);
		cv.wait(lck);
#endif
#endif
	}
	void notify_one()
	{
#ifdef __WIN32
		WakeConditionVariable(&cv);
#else
#ifdef __linux__
		pthread_cond_signal(&cv);
#else
		cv.notify_one();
#endif
#endif
	}
	void notify_all()
	{
#ifdef __WIN32
		WakeAllConditionVariable(&cv);
#else
#ifdef __linux__
		pthread_cond_broadcast(&cv);
#else
		cv.notify_all();
#endif
#endif
	}

private:
#ifdef __WIN32
	CONDITION_VARIABLE cv;
#else
#ifdef __linux__
	pthread_cond_t cv;
#else
	std::condition_variable cv;
#endif
#endif
};

#endif // !CRITICAL_SECTION_H
