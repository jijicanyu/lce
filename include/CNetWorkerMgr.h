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

template<class T>
class CNetWorkerMgr
{
public:
	int init(const string& sIp,const uint16_t wPort,uint32_t dwThreadNum,uint32_t dwMaxClient);

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
		return m_oEvent.run();
	}

	const char * getErrMsg(){ return m_szErrMsg;}

private:
    static void onAccept(int iFd,void *pData);


private:
	static CNetWorkerMgr<T> *m_pInstance;

	CEvent m_oEvent;
	char m_szErrMsg[1024];
	uint32_t m_dwMaxClient;
	uint32_t m_dwClientNum;
	vector<CNetWorker*> vecWorkers;
};

template<class T>
int CNetWorkerMgr<T>::init(const string& sIp,const uint16_t wPort,uint32_t dwThreadNum,uint32_t dwMaxClient)
{

	m_dwMaxClient = dwMaxClient;

	m_oEvent.init();

	int iFd = createTcpSock();
	if(iFd < 0)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
		return -1;
	}

	lce::setReUseAddr(iFd);
	lce::setNBlock(iFd);

	if(bind(iFd,sIp,wPort) < 0)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
		lce::close(iFd);
		return -1;
	}

	if(listen(iFd) < 0)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
		lce::close(iFd);
		return -1;
	}

	if(m_oEvent.addFdEvent(iFd,CEvent::EV_READ,CNetWorkerMgr::onAccept,NULL) < 0 )
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,m_oEvent.getErrorMsg());
		lce::close(iFd);
		return -1;
	}

	for(int i=0;i<dwThreadNum;i++)
	{
		CNetWorker * poWorker = new T;
		vecWorkers.push_back(poWorker);
		poWorker->init();
		poWorker->start();
	}

}

template<class T>
void CNetWorkerMgr<T>::onAccept(int iFd,void *pData)
{

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
			}
			break;
		}


		/*
		if(CNetWorkerMgr<T>::getInstance().m_dwClientNum > CNetWorkerMgr<T>::getInstance().m_dwMaxClient)
		{
			snprintf(CNetWorkerMgr<T>::getInstance().m_szErrMsg,sizeof(CNetWorkerMgr<T>::getInstance().m_szErrMsg),"onAccept %s,%d,max clients errno=%d,msg=%s",__FILE__,__LINE__,errno,strerror(errno));
			lce::close(iClientSock);
			continue;
		}
		*/

		CNetWorkerMgr<T>::getInstance().m_dwClientNum++;
		lce::setReUseAddr(iClientSock);
		lce::setNBlock(iClientSock);

		SClientInfo *pstClientInfo = new SClientInfo;

		pstClientInfo->iFd = iClientSock;
		pstClientInfo->stClientAddr = stClientAddr;


		CNetWorkerMgr<T>::getInstance().vecWorkers[CNetWorkerMgr<T>::getInstance().m_dwClientNum % CNetWorkerMgr<T>::getInstance().vecWorkers.size()]->watch(iClientSock,pstClientInfo);
	}

}

};

#endif