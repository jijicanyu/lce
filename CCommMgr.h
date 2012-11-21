#ifndef __NCE_CCOMMGR_H
#define __NCE_CCOMMGR_H

#include <string>
#include <vector>
#include <iostream>
#include "CSocketBuf.h"
#include "CPackageFilter.h"
#include "CEvent.h"
#include "Utils.h"
#include "signal.h"
#include "CProcessor.h"
using namespace std;

namespace lce
{

struct SSession
{
	SSession(){		memset(this,0,sizeof(SSession));	}
	std::string getStrIp()	{	return inet_ntoa(stClientAddr.sin_addr);	}
	uint32_t getIp()	const {	return ntohl(stClientAddr.sin_addr.s_addr);	}
	uint16_t getPort()	{	return ntohs(stClientAddr.sin_port);	}
	int iSvrId;
	int iFd;
    int iType;						//tcp use
	struct sockaddr_in stClientAddr;
	void *pData;					//由上层使用，会原值返回
	time_t dwBeginTime;
	time_t getDelayTime()
	{
		time_t dwCurTime = lce::getTickCount();
		if (dwCurTime >= dwBeginTime)
        {
			return dwCurTime - dwBeginTime;
		}
		else
        {
			return (time_t)(-1) - dwBeginTime + dwCurTime;
		}
		return 0;
	}
};


class CCommMgr
{
public:
    enum APP_TYPE
    {
        SRV_TCP=1,
        SRV_UDP=2,
        CONN_TCP=3,
    };

private:

    struct SServerInfo
    {
        SServerInfo()
        {
            pProcessor=NULL;
            pPackageFilter=NULL;
        }
        int iSrvId;
        int iFd;
        string sIp;
        uint16_t wPort;
		CProcessor *pProcessor;
        uint32_t dwInitRecvBufLen;
        uint32_t dwMaxRecvBufLen;
        uint32_t dwInitSendBufLen;
        uint32_t dwMaxSendBufLen;
        CPackageFilter *pPackageFilter;
        int iType;
    };

    struct SClientInfo
    {
        SClientInfo(){ memset(this,0,sizeof(SClientInfo));}
        int iSrvId;
        int iFd;
        bool bNeedClose;
        CSocketBuf *pSocketRecvBuf;
        CSocketBuf *pSocketSendBuf;
        sockaddr_in stClientAddr;
        ~SClientInfo()
        {
            if (pSocketRecvBuf != NULL) { delete pSocketRecvBuf;pSocketRecvBuf = NULL; }
            if (pSocketSendBuf != NULL) { delete pSocketSendBuf;pSocketSendBuf = NULL; }
        }
    };
	struct SProcessor
	{
		CProcessor *pProcessor;
		void * pData;
	};

public:
    int init()
    {
        if(m_oCEvent.init()< 0)
        {
            snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
            return -1;
        }
        m_vecClients.resize(EPOLL_MAX_SIZE,0);
        return 0;
    }

    int createSrv(int iType,const string &sIp,uint16_t wPort,uint32_t dwInitRecvBufLen,uint32_t dwMaxRecvBufLen,uint32_t dwInitSendBufLen,uint32_t dwMaxSendBufLen);
    int createAsyncConn(int iType,uint32_t dwInitRecvBufLen,uint32_t dwMaxRecvBufLen,uint32_t dwInitSendBufLen,uint32_t dwMaxSendBufLen);
    int setProcessor(int iSrvId,CProcessor * pProcessor,CPackageFilter * pPackageFilter);



    int close(const SSession &stSession);
    int write(const SSession &stSession,const char* pszData, const int iSize,bool bClose);


    int writeTo(const int iSrvId, const string& sIp, const uint16_t wPort, const char* pszData, const int iSize);

    int connect(int iSrvId,const string &sIp,uint16_t wPort);


    int addTimer(uint32_t dwTimerId,uint32_t dwExpire,CProcessor * pProcessor,void *pData);
    int delTimer(uint32_t dwTimerId);

    int addSigHandler(int iSignal,CProcessor * pProcessor);

    int start();
    int stop();
    int sendMessage(uint32_t dwMsgType,CProcessor * pProcessor,void* pData);

    const char * getErrMsg(){ return m_szErrMsg;}

    ~CCommMgr();

public:

    static void onWrite(int iFd,void *pData);
    static void onTcpRead(int iFd,void *pData);
    static void onUdpRead(int iFd,void *pData);

    static void onConnect(int iFd,void *pData);

    static void onAccept(int iFd,void *pData);
    static void onTimer(uint32_t dwTimerId,void *pData);
	static void onMessage(uint32_t dwMsgType,void *pData);
	static void onSignal(int iSignal);

    static CCommMgr & getInstance()
    {

        if (NULL == m_pInstance)
        {
			m_pInstance = new CCommMgr;
		}
		return *m_pInstance;
    }
private:
    int close(int iFd);
    int write(int iFd);
    inline bool isClose(int iFd);
    CCommMgr(){ }
private:
    vector <SServerInfo *> m_vecServers;
    vector <SClientInfo *> m_vecClients;
	map<int,SProcessor *> m_mapTimeProcs;
	map<int,CProcessor*> m_mapSigProcs;
    CEvent m_oCEvent;
    char m_szErrMsg[1024];
    static CCommMgr *m_pInstance;
};
};

#endif
