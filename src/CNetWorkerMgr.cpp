#include "CNetWorkerMgr.h"

namespace lce
{

CNetWorkerMgr* CNetWorkerMgr::m_pInstance = NULL;

template<class T>
int CNetWorkerMgr::init(const string& sIp,const uint16_t wPort,uint32_t dwThreadNum,uint32_t dwMaxClient)
{

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
		poWorker->start();
	}

}

void CNetWorkerMgr::onAccept(int iFd,void *pData)
{

	while(true) //循环接受请求，减小epoll软中断次数，提高性能
	{
		struct sockaddr_in stClientAddr;
		int iAddrLen = sizeof(struct sockaddr_in);

		int iClientSock = ::accept(iFd, (struct sockaddr *) &stClientAddr,(socklen_t *)&iAddrLen);

		SSession stSession;
		stSession.iSvrId = 0;

		if (iClientSock < 0)
		{
			if(errno != EAGAIN && errno != EINTR) //Resource temporarily unavailable
			{
				snprintf(CNetWorkerMgr::getInstance().m_szErrMsg,sizeof(CNetWorkerMgr::getInstance().m_szErrMsg),"onAccept %s,%d,accept errno=%d,msg=%s",__FILE__,__LINE__,errno,strerror(errno));
			}
			break;
		}

		stSession.ddwBeginTime=lce::getTickCount();

		if(CNetWorkerMgr::getInstance().m_dwClientNum > CNetWorkerMgr::getInstance().m_dwMaxClient)
		{
			snprintf(CNetWorkerMgr::getInstance().m_szErrMsg,sizeof(CNetWorkerMgr::getInstance().m_szErrMsg),"onAccept %s,%d,max clients errno=%d,msg=%s",__FILE__,__LINE__,errno,strerror(errno));
			lce::close(iClientSock);
			continue;
		}
	
		CNetWorkerMgr::getInstance().m_dwClientNum++;
		lce::setReUseAddr(iClientSock);
		lce::setNBlock(iClientSock);

		SClientInfo *pstClientInfo = new SClientInfo;

		pstClientInfo->iFd = iClientSock;
		pstClientInfo->stClientAddr = stClientAddr;


		vecWorkers[iClientSock%vecWorkers.size()].watch(iClientSock,pstClientInfo);
	}

}

};
