#ifndef __NCE_NETWORKER_MGR_H
#define __NCE_NETWORKER_MGR_H

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
#include "CThread.h"
#include "CNetWorker.h"


using namespace std;

namespace lce
{
typedef void (*NWMGR_ERROR_HANDLER)(char *szErrMsg);

template<class T>
class CNetWorkerMgr
{

public:
	int init(uint32_t dwThreadNum = 1,uint32_t dwMaxClient = 10000);

	static CNetWorkerMgr<T> & getInstance()
	{

		if (NULL == m_pInstance)
		{
			m_pInstance = new CNetWorkerMgr<T>;
		}
		return *m_pInstance;
	}

	int start()
	{
		for(size_t i=0;i<m_vecWorkers.size();++i)
		{
			m_vecWorkers[i]->start();
		}
		return m_oEvent.run();
	}

	int stop()
	{
		for(size_t i=0;i<m_vecWorkers.size();++i)
		{
			m_vecWorkers[i]->stop();
		}
		return m_oEvent.stop();
	}

public:
	int createSrv(const string &sIp,uint16_t wPort,int iPkgType = PKG_RAW,uint32_t dwInitRecvBufLen =10240,uint32_t dwMaxRecvBufLen=102400,uint32_t dwInitSendBufLen=102400,uint32_t dwMaxSendBufLen=1024000);
	int createAsyncConn(int iPkgType = PKG_RAW,uint32_t dwInitRecvBufLen =10240,uint32_t dwMaxRecvBufLen=102400,uint32_t dwInitSendBufLen=102400,uint32_t dwMaxSendBufLen=1024000);
	int setPkgFilter(int iSrvId,CPackageFilter *pPkgFilter);
	int setErrHandler(NWMGR_ERROR_HANDLER pErrHandler = NULL);
	const char * getErrMsg(){ return m_szErrMsg;}

private:

	CNetWorkerMgr(){}

    static void onAccept(int iFd,void *pData);


private:
	static CNetWorkerMgr<T> *m_pInstance;
	NWMGR_ERROR_HANDLER m_pErrHandler;
	CEvent m_oEvent;
	char m_szErrMsg[1024];
	uint32_t m_dwMaxClient;
	uint32_t m_dwClientNum;
	vector<CNetWorker*> m_vecWorkers;
	vector <SServerInfo *> m_vecServers;

};

template<class T>
int CNetWorkerMgr<T>::init(uint32_t dwThreadNum,uint32_t dwMaxClient)
{

	if(m_oEvent.init(dwMaxClient * FD_TIMES)< 0)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
		return -1;
	}

	m_dwMaxClient = dwMaxClient;
	m_pErrHandler = NULL;

	for(int i=0;i<dwThreadNum;i++)
	{
		CNetWorker * poWorker = new T;
		m_vecWorkers.push_back(poWorker);
		poWorker->init(dwMaxClient * FD_TIMES);
	}

}

template<class T>
int CNetWorkerMgr<T>::createSrv(const string &sIp,uint16_t wPort,int iPkgType,uint32_t dwInitRecvBufLen,uint32_t dwMaxRecvBufLen,uint32_t dwInitSendBufLen,uint32_t dwMaxSendBufLen)
{

  	SServerInfo *pstServerInfo = new SServerInfo;

	pstServerInfo->sIp=sIp;
	pstServerInfo->wPort=wPort;
	pstServerInfo->iType=SRV_TCP;

	pstServerInfo->dwInitRecvBufLen=dwInitRecvBufLen;
	pstServerInfo->dwMaxRecvBufLen=dwMaxRecvBufLen;

	pstServerInfo->dwInitSendBufLen=dwInitSendBufLen;
	pstServerInfo->dwMaxSendBufLen=dwMaxSendBufLen;

	int iFd = lce::createTcpSock();
	if(iFd < 0)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
		delete pstServerInfo;
		return -1;
	}

	lce::setReUseAddr(iFd);
	lce::setNBlock(iFd);

	if(lce::bind(iFd,sIp,wPort) < 0)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
		lce::close(iFd);
		delete pstServerInfo;
		return -1;
	}

	if(lce::listen(iFd) < 0)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
		lce::close(iFd);
		delete pstServerInfo;
		return -1;
	}

	pstServerInfo->iFd=iFd;

	switch(iPkgType)
	{
	case PKG_RAW:
		pstServerInfo->iPkgType = iPkgType;
		pstServerInfo->pPackageFilter = new CRawPackageFilter;
		break;
	case PKG_HTTP:
		pstServerInfo->iPkgType = iPkgType;
		pstServerInfo->pPackageFilter = new CHttpPackageFilter;
		break;
	case PKG_H2ST3:
		pstServerInfo->iPkgType = iPkgType;
		pstServerInfo->pPackageFilter = new CH2ShortT3PackageFilter;
		break;
	case PKG_H2LT3:
		pstServerInfo->iPkgType = iPkgType;
		pstServerInfo->pPackageFilter = new CH2T3PackageFilter;
		break;
	case PKG_EXT:
		pstServerInfo->iPkgType = iPkgType;
		pstServerInfo->pPackageFilter = NULL;
		break;
	default:
		pstServerInfo->iPkgType = PKG_RAW;
		pstServerInfo->pPackageFilter = new CRawPackageFilter;

	}

	if(m_oEvent.addFdEvent(pstServerInfo->iFd,CEvent::EV_READ,CNetWorkerMgr<T>::onAccept,pstServerInfo) < 0 )
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,m_oEvent.getErrorMsg());
		lce::close(iFd);
		delete pstServerInfo;
		return -1;
	}

	pstServerInfo->iSrvId = m_vecServers.size();
	m_vecServers.push_back(pstServerInfo);
	return pstServerInfo->iSrvId;

}

template<class T>

int CNetWorkerMgr<T>::setErrHandler(NWMGR_ERROR_HANDLER pErrHandler /* = NULL */)
{
	m_pErrHandler = pErrHandler;
}

template<class T>
int CNetWorkerMgr<T>::setPkgFilter(int iSrvId,CPackageFilter *pPkgFilter)
{
	if( iSrvId <0 || iSrvId >(int) m_vecServers.size()-1 )
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,iSrvId error",__FILE__,__LINE__);
		return -1;
	}

	if(pPkgFilter == NULL)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,pkgfilter pointer is null",__FILE__,__LINE__);
		return -1;
	}

	SServerInfo * pstServerInfo = m_vecServers[iSrvId];

	if (pstServerInfo == NULL)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,iSrvId error",__FILE__,__LINE__);
		return -1;
	}

	pstServerInfo->pPackageFilter = pPkgFilter;
	return 0;
}



template<class T>
void CNetWorkerMgr<T>::onAccept(int iFd,void *pData)
{
	SServerInfo *pstServerInfo=(SServerInfo *)pData;

	if (pstServerInfo == NULL) return;

	while(true) //循环接受请求，减小epoll软中断次数，提高性能
	{
		struct sockaddr_in stClientAddr;
		int iAddrLen = sizeof(struct sockaddr_in);

		int iClientSock = ::accept(iFd, (struct sockaddr *) &stClientAddr,(socklen_t *)&iAddrLen);


		if (iClientSock < 0)
		{
			if(errno != EAGAIN && errno != EINTR) //Resource temporarily unavailable
			{
				snprintf(CNetWorkerMgr<T>::getInstance().m_szErrMsg,sizeof(CNetWorkerMgr<T>::getInstance().m_szErrMsg),"onAccept %s,%d,accept errno=%d,msg=%s",__FILE__,__LINE__,errno,strerror(errno));
				
				if(CNetWorkerMgr<T>::getInstance().m_pErrHandler != NULL)
					CNetWorkerMgr<T>::getInstance().m_pErrHandler(CNetWorkerMgr<T>::getInstance().m_szErrMsg);
			}
			break;
		}

		
		if(iClientSock > CNetWorkerMgr<T>::getInstance().m_dwMaxClient)
		{
			snprintf(CNetWorkerMgr<T>::getInstance().m_szErrMsg,sizeof(CNetWorkerMgr<T>::getInstance().m_szErrMsg),"onAccept %s,%d,max clients errno=%d,msg=%s",__FILE__,__LINE__,errno,strerror(errno));
			if(CNetWorkerMgr<T>::getInstance().m_pErrHandler != NULL)
				CNetWorkerMgr<T>::getInstance().m_pErrHandler(CNetWorkerMgr<T>::getInstance().m_szErrMsg);

			lce::close(iClientSock);
			continue;
		}
		

		CNetWorkerMgr<T>::getInstance().m_dwClientNum++;
		lce::setReUseAddr(iClientSock);
		lce::setNBlock(iClientSock);

		SClientInfo *pstClientInfo = new SClientInfo;
		pstClientInfo->iFd = iClientSock;
		pstClientInfo->stClientAddr = stClientAddr;
		pstClientInfo->pstServerInfo = pstServerInfo;

		int iIndex = CNetWorkerMgr<T>::getInstance().m_dwClientNum % CNetWorkerMgr<T>::getInstance().m_vecWorkers.size();
		pstClientInfo->poWorker = CNetWorkerMgr<T>::getInstance().m_vecWorkers[iIndex];
		
		pstClientInfo->poWorker->watch(iClientSock,pstClientInfo);
	}
}


};

#endif