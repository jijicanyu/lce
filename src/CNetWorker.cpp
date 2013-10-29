#include "CNetWorker.h"

namespace lce
{

	int CNetWorker::init()
	{
		m_queClients.init();
		m_iEventFd = eventfd(0, EFD_NONBLOCK) ;
		m_oEvent.addFdEvent(m_iEventFd,CEvent::EV_READ,CNetWorker::onQueueEvent,this);
	}

	void CNetWorker::run()
	{
		m_oEvent.run();
	}

	void CNetWorker::watch(int iFd,void *pData)
	{
		SSession stSession;
		onConnect(stSession,true);
		uint64_t ddwCount = 1;
		m_queClients.push(pData);
		write(m_iEventFd,&ddwCount,sizeof(ddwCount));
	}

	void CNetWorker::onQueueEvent(int iFd,void *pData)
	{
		CNetWorker *poWorker = (CNetWorker)pData;
		uint64_t ddwCount = 0;
		read(m_iEventFd,ddwCount,sizeof(uint64_t));
		SClientInfo *pstClientInfo = NULL;
		poWorker->m_queClients.pop(pstClientInfo);

		poWorker->m_oEvent.addFdEvent(pstClientInfo->iFd,CEvent::EV_READ,CNetWorker::onTcpRead,this);
	}


	void CNetWorker::onTcpRead(int iFd,void *pData)
	{

		CNetWorker *poWorker = (CNetWorker)pData;

		SSession stSession;
		char szBuf[65535] = {0};
		int iSize = ::recv(iFd,szBuf,sizeof(szBuf),0);
		if(iSize == 0)
		{
			poWorker->onClose(stSession);
		}
		else if(iSize < 0)
		{
			
		}
		else
		{
			poWorker->onRead(stSession,szBuf,iSize);
		}

	}

};