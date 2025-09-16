#define NOMINMAX
#include "tjsCommHead.h"
#include "ThreadIntf.h"
#include "MsgIntf.h"

#include <thread>
#include <pthread.h>

class tTVPNativeThread : public tTVPNativeThreadIntf
{
public:
  tTVPNativeThread();
	virtual ~tTVPNativeThread();
	virtual void Start(tTVPThreadFunc func, void *arg, tTVPThreadPriority pri);
	virtual void WaitFor();
	virtual void SetPriority(tTVPThreadPriority pri);
	virtual void SetProcessorNo(int no);
private:
	int calcPriority(tTVPThreadPriority pri);
	std::thread* Thread;
	std::thread::native_handle_type GetHandle() { return Thread ? Thread->native_handle() : 0; }
	static unsigned ThreadFunc(void *arg);
	static std::vector<tjs_int> processor_ids;
};

std::vector<tjs_int> tTVPNativeThread::processor_ids;

tTVPNativeThread::tTVPNativeThread() 
: Thread(nullptr)
{        
}

tTVPNativeThread::~tTVPNativeThread()
{
	if ( Thread != nullptr ) {
		if( Thread->joinable() ) Thread->detach();
		delete Thread;
	}
};

void 
tTVPNativeThread::Start(tTVPThreadFunc func, void *arg, tTVPThreadPriority pri)
{
	if( Thread == nullptr ) {
	    Thread = new std::thread( func, arg );
    	SetPriority(pri);
	}
}

void
tTVPNativeThread::WaitFor()
{
	if (Thread && Thread->joinable()) { 
		Thread->join(); 
	}    
}

int tTVPNativeThread::calcPriority(tTVPThreadPriority pri)
{
	int policy;
	struct sched_param param;
	int err = pthread_getschedparam( GetHandle(), &policy, &param );
	int maxpri = sched_get_priority_max( policy );
	int minpri = sched_get_priority_min( policy );
	int range = (maxpri - minpri);
	int half = range / 2;

	int npri = minpri + half;
	switch(pri)
	{
	case ttpIdle:			npri = minpri;						break;
	case ttpLowest:			npri = minpri + half/3;				break;
	case ttpLower:			npri = minpri + (half*2)/3;			break;
	case ttpNormal:			npri = minpri + half;				break;
	case ttpHigher:			npri = minpri + half + half/3;		break;
	case ttpHighest:		npri = minpri + half+ (half*2)/3;	break;
	case ttpTimeCritical:	npri = maxpri;						break;
	}
	return npri;
}


void 
tTVPNativeThread::SetPriority(tTVPThreadPriority pri)
{
	if (Thread) {
		int npri = calcPriority(pri);
		int policy;
		struct sched_param param;
		int err = pthread_getschedparam( GetHandle(), &policy, &param );
		param.sched_priority = npri;
		err = pthread_setschedparam( GetHandle(), policy, &param );
	}
}

void
tTVPNativeThread::SetProcessorNo(int no)
{
	cpu_set_t cpuset;
	CPU_ZERO( &cpuset );
	CPU_SET( no, &cpuset );
	int rc = pthread_setaffinity_np( GetHandle(), sizeof( cpu_set_t ), &cpuset );
}

unsigned tTVPNativeThread::ThreadFunc(void *arg)
{
	tTVPThread *self = (tTVPThread*)arg;
	self->StartProc();
	return 0;
}

void 
TVPYieldNativeThread(int Milliseconds)
{
	if (Milliseconds == 0) {
		std::this_thread::yield();
	} else if (Milliseconds > 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(Milliseconds));
	}
}

tTVPNativeThreadIntf *
TVPCreateNativeThread()
{
    return new tTVPNativeThread();
}

tjs_int TVPGetProcessorNum( void )
{
	return std::thread::hardware_concurrency();
}
