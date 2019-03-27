#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "CTimer.h"

 
Timer::Timer(TimerManager& manager)
    : manager_(manager)
    , heapIndex_(-1)
{
}
 
Timer::~Timer()
{
    Stop();
}
 
void Timer::Stop()
{
    if ( heapIndex_ != -1 )
    {
        manager_.RemoveTimer(this);
        heapIndex_ = -1;
    }
}
 
void Timer::OnTimer(unsigned long long now)
{
    if (timerType_ == Timer::CIRCLE)
    {
        expires_ = interval_ + now;
        manager_.AddTimer(this);
    }
    else
    {
        heapIndex_ = -1;
    }
	//printf("printf,TimerHandler\n");
	timerFun_(timerFunParm_);
}
 
//////////////////////////////////////////////////////////////////////////
// TimerManager
 

TimerManager::TimerManager(int miniseconds):m_handle(0),m_stop_flag(0),m_interval(miniseconds)
{
}

TimerManager::~TimerManager()
{
	StopTimerManager();
}

bool TimerManager::StartTimerManager()
{
	StopTimerManager();
	::InterlockedExchange( &m_stop_flag , 0 );
	 bool ret = false;  
#ifdef WIN32  
    m_handle = (HANDLE)_beginthreadex(NULL, 0, thread_func, this, 0, NULL);  
    if (NULL != m_handle)  
    {  
        ret = true;  
    }  
#else  
    if (0 == pthread_create(&m_thread_t, NULL, thread_func, this))  
    {  
        ret = true;  
    }  
    else  
    {  
        m_thread_t = 0;  
    }  
#endif  
    return ret; 
}

//线程函数 
#ifdef WIN32  
unsigned __stdcall TimerManager::thread_func(void* arg)  
#else  
void* TimerManager::thread_func(void* arg)  
#endif  
{  
	if( NULL != arg )
	{
		TimerManager *pthis = (TimerManager*)arg;  
		pthis->run();  
	}
    return NULL;  
} 

void TimerManager::run()
{
	printf("begin Timer Manager at %d\n", GetCurrentThreadId());
	while( 0 == m_stop_flag )
	{
		//定期检测 timer 的超时
		DetectTimers();
		Sleep(m_interval);
	}
	printf("\nend Timer Manager at %d\n",GetCurrentThreadId());
}

void TimerManager::StopTimerManager()
{
	::InterlockedIncrement(&m_stop_flag);
	if( 0 != m_handle )
	{
		if( ::WaitForSingleObject( m_handle, m_interval*2 ) == WAIT_TIMEOUT ) 
			TerminateThread( m_handle ,2) ;
		m_handle = 0;
	}
}


void TimerManager::AddTimer(Timer* timer)
{
    timer->heapIndex_ = heap_.size();
    HeapEntry entry = { timer->expires_, timer };
    heap_.push_back(entry);
    UpHeap(heap_.size() - 1);
}
 
void TimerManager::RemoveTimer(Timer* timer)
{
    size_t index = timer->heapIndex_;
    if (!heap_.empty() && index < heap_.size())
    {
        if (index == heap_.size() - 1)
        {
            heap_.pop_back();
        }
        else
        {
            SwapHeap(index, heap_.size() - 1);
            heap_.pop_back();
            size_t parent = (index - 1) / 2;
            if (index > 0 && heap_[index].time < heap_[parent].time)
                UpHeap(index);
            else
                DownHeap(index);
        }
    }
}
 
void TimerManager::DetectTimers()
{
    unsigned long long now = GetCurrentMillisecs();
 
    while (!heap_.empty() && heap_[0].time <= now)
    {
        Timer* timer = heap_[0].timer;
        RemoveTimer(timer);
        timer->OnTimer(now);
    }
}
 
void TimerManager::UpHeap(size_t index)
{
    size_t parent = (index - 1) / 2;
    while (index > 0 && heap_[index].time < heap_[parent].time)
    {
        SwapHeap(index, parent);
        index = parent;
        parent = (index - 1) / 2;
    }
}
 
void TimerManager::DownHeap(size_t index)
{
    size_t child = index * 2 + 1;
    while (child < heap_.size())
    {
        size_t minChild = (child + 1 == heap_.size() || heap_[child].time < heap_[child + 1].time)
            ? child : child + 1;
        if (heap_[index].time < heap_[minChild].time)
            break;
        SwapHeap(index, minChild);
        index = minChild;
        child = index * 2 + 1;
    }
}
 
void TimerManager::SwapHeap(size_t index1, size_t index2)
{
    HeapEntry tmp = heap_[index1];
    heap_[index1] = heap_[index2];
    heap_[index2] = tmp;
    heap_[index1].timer->heapIndex_ = index1;
    heap_[index2].timer->heapIndex_ = index2;
}
 
 
unsigned long long TimerManager::GetCurrentMillisecs()
{
#ifdef _MSC_VER
    _timeb timebuffer;
    _ftime(&timebuffer);
    unsigned long long ret = timebuffer.time;
    ret = ret * 1000 + timebuffer.millitm;
    return ret;
#else
    timeval tv;         
    ::gettimeofday(&tv, 0);
    unsigned long long ret = tv.tv_sec;
    ret = ret * 1000 + tv.tv_usec / 1000;
#endif
}


//为何在此定义，因为头文件会被其他文件包含，而此文件同样包含了他的头文件，会产生重复定义
//在需要使用该实例  的 cpp  文件里面 最头部添加  extern _Log::_MyLog OutLog;
TimerManager g_timer_manager( 100 );