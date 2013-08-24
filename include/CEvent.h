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
#include<tr1/unordered_map>
#include <algorithm>
#include <fcntl.h>
#include <iostream>
using namespace std;

namespace lce
{
typedef void (*FdEventCb)(int iFd,void *pData);
typedef void (*TimeEventCb)(int iTimeId,void *pData);
typedef void (*MsgEventCb)(int iMsgType,void *pData);

const int EPOLL_MAX_EVENT = 1024;
const int EPOLL_WAIT_TIMEOUT = 3000;

class CEvent
{
public:

	typedef tr1::unordered_map<int,uint64_t> MAP_TIME_INDEX;

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
        FdEventCb pReadProc;
        FdEventCb pWriteProc;
        void *pClientRData;
        void *pClientWData;
    } SFdEvent;

    typedef struct
    {
        int iTimerId;
        uint64_t ddwMillSecs;
        TimeEventCb pTimeProc;
        void *pClientData;
    } STimeEvent;

    typedef struct
    {
        int iMsgType;
        MsgEventCb pMsgProc;
        void *pClientData;
    } SMsgEvent;


    struct cmpTimer
    {
        bool operator () (const STimeEvent &stTimeEvent1,const STimeEvent &stTimeEvent2)
        {
            return stTimeEvent1.ddwMillSecs < stTimeEvent2.ddwMillSecs;
        }

    };


public:
    CEvent();
    ~CEvent();

    int init(uint32_t dwMaxFdNum = 10000);
    int addFdEvent(int iWatchFd,int iEventType,FdEventCb pFdCb,void * pClientData);
    int delFdEvent(int iWatchFd,int iEventType);

    int addTimer(int iTimerId,uint32_t dwExpire,TimeEventCb pTimeCb,void * pClientData);
    int delTimer(int iTimerId);
    int addMessage(int iMsgType,MsgEventCb pMsgCb,void * pClientData);

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
    queue <SMsgEvent> m_queMsgEvents;
    multiset <STimeEvent,cmpTimer> m_setSTimeEvents;
	MAP_TIME_INDEX m_mapTimeEventIndexs;
    char m_szErrMsg[1024];
    bool m_bRun;
	bool m_bInit;
	uint32_t m_dwMaxFdNum;
    pthread_mutex_t  m_lock;

};
};
#endif
