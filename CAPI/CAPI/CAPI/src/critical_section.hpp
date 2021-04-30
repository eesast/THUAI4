#ifndef CRITICAL_SECTION_H

#define CRITICAL_SECTION_H

#include<Winsock2.h>
#include<Windows.h>

#include<numeric>

//WRL里应该有，但我头文件没找到……
//还想支持Linux

class ConditionVariable;

class CriticalSection {
public:
	CriticalSection() { InitializeCriticalSection(&cs); }
	~CriticalSection() { DeleteCriticalSection(&cs); }
	void lock() { EnterCriticalSection(&cs); }
	void unlock() { LeaveCriticalSection(&cs); }
	bool try_lock() { return TryEnterCriticalSection(&cs); }
private:
	CRITICAL_SECTION cs;
	friend class ConditionVariable;
};

class ConditionVariable {
public:
	ConditionVariable() { InitializeConditionVariable(&cv); }
	~ConditionVariable() = default;
	void wait(CriticalSection& cs) { SleepConditionVariableCS(&cv, &cs.cs, std::numeric_limits<DWORD>::infinity()); }
	void notify_one() { WakeConditionVariable(&cv); }
	void notify_all() { WakeAllConditionVariable(&cv); }
private:
	CONDITION_VARIABLE cv;
};


#endif // !CRITICAL_SECTION_H

