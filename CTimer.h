/*  用法与用量

static void TimerHandler1(void* p)
{
    printf("TimerHandler11111  x = %d\n",*(int*)p );
}
static void TimerHander2(void* p)
{
    printf("Timer22\n");
}

	TimerManager tm( 100 );
	tm.StartTimerManager();
    Timer t(tm);
	Timer t2(tm);

	int x = 0;
    t.Start(TimerHandler1, 1000, &x);
	t2.Start(TimerHander2,1333);

	//停止
	tm.StopTimerManager();
*/

#pragma once 

#ifdef _MSC_VER 
# include <sys/timeb.h>
#include <process.h>
#include <Windows.h>

#else
#include <sys/time.h>
#include <pthread.h>  
#endif

#include <vector>

//定时器回调函数定义
typedef   void(* TimerFunction)(void* p) ; 


class TimerManager;
 
class Timer
{
public:
    enum TimerType { ONCE, CIRCLE };
 
    Timer(TimerManager& manager);
    ~Timer();
 
    //template<typename Fun>
	void Start(TimerFunction fun, unsigned interval,  void* parm = NULL,TimerType timeType = CIRCLE);
    void Stop();

private:
    void OnTimer(unsigned long long now);
 
private:
    friend class TimerManager;
    TimerManager& manager_;
    TimerType timerType_;
    TimerFunction timerFun_;
	void *        timerFunParm_;
    unsigned interval_;
    unsigned long long expires_;
 
    size_t heapIndex_;
};


 
class TimerManager
{
public:
	TimerManager(int miniseconds = 50);
	~TimerManager();
    static unsigned long long GetCurrentMillisecs();
	
	void StopTimerManager();
	bool StartTimerManager();

	void run();
	//线程函数
#ifdef WIN32  
    static unsigned __stdcall thread_func(void* arg);  
#else  
    static void* thread_func(void* arg);  
#endif  
 
private:

    friend class Timer;
    void AddTimer(Timer* timer);
    void RemoveTimer(Timer* timer);
    
 
    void UpHeap(size_t index);
    void DownHeap(size_t index);
    void SwapHeap(size_t, size_t index2);
 
private:
	void DetectTimers();	 
	int							m_interval;
	volatile unsigned long		m_stop_flag;
	HANDLE						m_handle;
    struct HeapEntry
    {
        unsigned long long time;
        Timer* timer;
    };
    std::vector<HeapEntry>		heap_;
};
 


//template<typename Fun>
inline void Timer::Start(TimerFunction fun, unsigned interval, void* parm, TimerType timeType)
{
    Stop();
    interval_ = interval;
    timerFun_ = fun;
    timerType_ = timeType;
	timerFunParm_ = parm;
    this->expires_ = this->interval_ + TimerManager::GetCurrentMillisecs();
    manager_.AddTimer(this);
}
 