#ifndef __NCE_CEVENT_H
#define __NCE_CEVENT_H

#include <sys/epoll.h>
#include <stddef.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <queue>
#include <set>
#include <hash_map>
#include <algorithm>
#include <fcntl.h>

using namespace __gnu_cxx;
using namespace std;

namespace lce
{
typedef void (*fdEventCb)(int iFd,void *pData);
typedef void (*timeEventCb)(uint32_t dwTimeId,void *pData);
typedef void (*msgEventCb)(uint32_t dwMsgType,void *pData);

const int EPOLL_MAX_EVENT = 1024;
const int EPOLL_WAIT_TIMEOUT = 3000;

class CEvent
{
public:

	typedef hash_map<uint32_t,uint64_t> MAP_TIME_INDEX;

    enum EventType
    {
        EV_DONE=0,
        EV_READ=1,
        EV_WRITE=2,

    };
    typedef struct
    {
        int iEPollFd;
        struct epoll_event stEvents[EPOLL_MAX_EVENT];
    } SEPollState;

    typedef struct
    {
        int iEventType;
        fdEventCb pReadProc;
        fdEventCb pWriteProc;
        void *pClientRData;
        void *pClientWData;
    } SFdEvent;

    typedef struct
    {
        uint32_t dwTimerId;
        uint64_t ddwMillSecs;
        timeEventCb pTimeProc;
        void *pClientData;
    } STimeEvent;

    typedef struct
    {
        uint32_t dwMsgType;
        msgEventCb pMsgProc;
        void *pClientData;
    } SMsgEvent;


    struct cmpTimer
    {
        bool operator () (const STimeEvent *stTimeEvent1,const STimeEvent *stTimeEvent2)
        {
            return stTimeEvent1->ddwMillSecs < stTimeEvent2->ddwMillSecs;
        }

    };


public:
    CEvent();
    ~CEvent();

    int init(uint32_t dwMaxFdNum = 100000);
    int addFdEvent(int iWatchFd,int iEventType,fdEventCb pFdCb,void * pClientData);
    int delFdEvent(int iWatchFd,int iEventType);

    int addTimer(uint32_t dwTimerId,uint32_t dwExpire,timeEventCb pTimeCb,void * pClientData);
    int delTimer(uint32_t dwTimerId);
    int addMessage(uint32_t dwMsgType,msgEventCb pMsgCb,void * pClientData);

    int run();
    int stop();
    const char * getErrorMsg(){ return m_szErrMsg; }
private:


    static inline uint64_t  getMillSecsNow()
    {
        struct timeval stNow;
        gettimeofday(&stNow, NULL);
        return (stNow.tv_sec*1000+stNow.tv_usec/1000);
    }

private:

    int m_iMsgFd[2];
    SFdEvent *m_stFdEvents;
    SEPollState m_stEPollState;
    queue <SMsgEvent *> m_queMsgEvents;
    multiset <STimeEvent*,cmpTimer> m_setSTimeEvents;
	MAP_TIME_INDEX m_mapTimeEventIndexs;
    char m_szErrMsg[1024];
    bool m_bRun;
	bool m_bInit;
	uint32_t m_dwMaxFdNum;
    pthread_mutex_t  m_lock;

};
};
#endif
