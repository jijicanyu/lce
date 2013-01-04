#ifndef __NCE_CCOMMGR_H
#define __NCE_CCOMMGR_H

#include <string>
#include <vector>
#include <iostream>
#include "Utils.h"
#include "CSocketBuf.h"
#include "CPackageFilter.h"
#include "CH2T3PackageFilter.h"
#include "CH2ShortT3PackageFilter.h"
#include "CHttpPackageFilter.h"
#include "CRawPackageFilter.h"
#include "CEvent.h"
#include "signal.h"
#include "CProcessor.h"
using namespace std;

namespace lce
{

const float MAGIC_FD_TIMES =  1.5;

struct SSession
{
	SSession(){		memset(this,0,sizeof(SSession));	}
	std::string getStrIp()	{	return inet_ntoa(stClientAddr.sin_addr);	}
	uint32_t getIp()	const {	return ntohl(stClientAddr.sin_addr.s_addr);	}
	uint16_t getPort()	{	return ntohs(stClientAddr.sin_port);	}
	int iSvrId;
	int iFd;
	void * pData;
	struct sockaddr_in stClientAddr;
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

	enum PKG_TYPE
	{
		PKG_RAW = 0,
		PKG_H2ST3 = 1,
		PKG_H2LT3 = 2,
		PKG_HTTP = 3,

	};

	enum ERR_TYPE
	{
		ERR_SOCKET = 1,
		ERR_INVALID_PACKAGE = 2,
		ERR_MAX_CLIENT = 3,
		ERR_NO_BUFFER = 4,
		ERR_NOT_READY = 5,
		ERR_SYSTEM = 6,
	};

private:

    struct SServerInfo
    {
        SServerInfo()
        {
            pProcessor=NULL;
            pPackageFilter=NULL;
			iPkgType = 0;
			iFd = 0;
			iSrvId = 0;
			iType = 0;
        }
        int iSrvId;
        int iFd;
        string sIp;
        uint16_t wPort;
		int iPkgType;
		CProcessor *pProcessor;
        uint32_t dwInitRecvBufLen;
        uint32_t dwMaxRecvBufLen;
        uint32_t dwInitSendBufLen;
        uint32_t dwMaxSendBufLen;
        CPackageFilter *pPackageFilter;
        int iType;

		~SServerInfo()
		{
			if(pPackageFilter != NULL) { delete pPackageFilter;pPackageFilter = NULL ;}
		}
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
    int init(uint32_t dwMaxClient = 5000)
    {
        if(m_oCEvent.init(dwMaxClient * MAGIC_FD_TIMES)< 0)
        {
            snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
            return -1;
        }
		m_dwMaxClient = dwMaxClient;
		m_dwClientNum = 0;
        m_vecClients.resize(dwMaxClient * MAGIC_FD_TIMES,0);
        return 0;
    }

    int createSrv(int iType,const string &sIp,uint16_t wPort,uint32_t dwInitRecvBufLen =10240,uint32_t dwMaxRecvBufLen=102400,uint32_t dwInitSendBufLen=102400,uint32_t dwMaxSendBufLen=1024000);
    int createAsyncConn(uint32_t dwInitRecvBufLen =10240,uint32_t dwMaxRecvBufLen=102400,uint32_t dwInitSendBufLen=102400,uint32_t dwMaxSendBufLen=1024000);
    int setProcessor(int iSrvId,CProcessor * pProcessor,int iPkgType = 0);
	int rmSrv(int iSrvId);


    int close(const SSession &stSession);
    int write(const SSession &stSession,const char* pszData, const int iSize,bool bClose = true);


    int writeTo(const int iSrvId, const string& sIp, const uint16_t wPort, const char* pszData, const int iSize);

    int connect(int iSrvId,const string &sIp,uint16_t wPort);


    int addTimer(uint32_t dwTimerId,uint32_t dwExpire,CProcessor * pProcessor,void *pData = NULL);
    int delTimer(uint32_t dwTimerId);

    int addSigHandler(int iSignal,CProcessor * pProcessor);

    int start();
    int stop();
	

    int sendMessage(uint32_t dwMsgType,CProcessor * pProcessor,void* pData = NULL);

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
	uint32_t m_dwMaxClient;
	uint32_t m_dwClientNum;
};
};

#endif
