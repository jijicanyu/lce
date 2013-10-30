#include "CNetWorker.h"

namespace lce
{

	int CNetWorker::init()
	{
		m_vecClients.resize(12000,0);
		m_oEvent.init();
		m_queClients.init();
		m_iEventFd = eventfd(0, EFD_NONBLOCK) ;
		m_oEvent.addFdEvent(m_iEventFd,CEvent::EV_READ,CNetWorker::onQueueEvent,this);
	}

	int CNetWorker::run()
	{
		m_oEvent.run();
	}

	int CNetWorker::watch(int iFd,void *pData)
	{
		uint64_t ddwCount = 1;
		m_queClients.push((SClientInfo*)pData);
		::write(m_iEventFd,&ddwCount,sizeof(ddwCount));
		
		return 0;
	}

	void CNetWorker::onQueueEvent(int iFd,void *pData)
	{
		CNetWorker *poWorker = (CNetWorker*)pData;
		uint64_t ddwCount = 0;
		::read(iFd,&ddwCount,sizeof(uint64_t));

		SClientInfo *pstClientInfo = NULL;

		while(poWorker->m_queClients.pop(&pstClientInfo))
		{
			poWorker->m_vecClients[pstClientInfo->iFd] = pstClientInfo;
			SSession stSession;
			stSession.iFd = pstClientInfo->iFd;
			stSession.stClientAddr = pstClientInfo->stClientAddr;
			poWorker->onConnect(stSession,true);

			poWorker->m_oEvent.addFdEvent(pstClientInfo->iFd,CEvent::EV_READ,CNetWorker::onTcpRead,poWorker);
		}
	}


	void CNetWorker::onTcpRead(int iFd,void *pData)
	{
		CNetWorker *poWorker = (CNetWorker*)pData;
		SClientInfo *pstClientInfo = poWorker->m_vecClients[iFd];

		SSession stSession;
		stSession.iFd = iFd;
		stSession.stClientAddr = pstClientInfo->stClientAddr;

		char szBuf[10240] = {0};
		int iSize = ::recv(iFd,szBuf,sizeof(szBuf),0);
		if(iSize == 0)
		{
			delete pstClientInfo;
			lce::close(iFd);
			poWorker->m_vecClients[iFd] = NULL;
			poWorker->onClose(stSession);
			poWorker->m_oEvent.delFdEvent(iFd,CEvent::EV_READ);

		}
		else if(iSize < 0)
		{
			delete pstClientInfo;
			lce::close(iFd);
			poWorker->m_vecClients[iFd] = NULL;
			poWorker->m_oEvent.delFdEvent(iFd,CEvent::EV_READ);

			cout<<"recv error"<<endl;
		}
		else
		{
			poWorker->onRead(stSession,szBuf,iSize);
		}

	}

	int CNetWorker::write(const SSession &stSession,char*pszData,int iSize,bool bClose /* = true */)
	{
		lce::send(stSession.iFd,pszData,iSize);
		lce::close(stSession.iFd);
		delete 	m_vecClients[stSession.iFd];
		m_vecClients[stSession.iFd] = NULL;
		m_oEvent.delFdEvent(stSession.iFd,CEvent::EV_READ);


	}

};