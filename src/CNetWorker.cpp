#include "CNetWorker.h"

namespace lce
{

int CNetWorker::init(uint32_t dwMaxClient /* = 10000 */)
{
	m_vecClients.resize(dwMaxClient*FD_TIMES,0);

	if(m_oEvent.init(dwMaxClient*FD_TIMES) < 0)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
		return -1;
	}

	return 0;
}

int CNetWorker::run()
{
	return m_oEvent.run();
}

int CNetWorker::watch(int iFd,void *pData)
{
	return m_oEvent.addFdEvent(iFd,CEvent::EV_READ,CNetWorker::onTcpRead,pData);
}


void CNetWorker::onTcpRead(int iFd,void *pData)
{

	SClientInfo *pstClientInfo = (SClientInfo*)pData;
	CNetWorker *poWorker = pstClientInfo->poWorker;

	poWorker->m_vecClients[iFd] = pstClientInfo;

	SSession stSession;
	stSession.ddwBeginTime = lce::getTickCount();
	stSession.iFd = iFd;
	stSession.iSvrId = pstClientInfo->pstServerInfo->iSrvId;
	stSession.stClientAddr = pstClientInfo->stClientAddr;

	if (pstClientInfo->pSocketRecvBuf == NULL)
	{
		pstClientInfo->pSocketRecvBuf = new CSocketBuf(pstClientInfo->pstServerInfo->dwInitRecvBufLen,pstClientInfo->pstServerInfo->dwMaxRecvBufLen);
	}

	int iSize = 0;

	if(pstClientInfo->pSocketRecvBuf->getFreeSize() == 0)
	{
		pstClientInfo->pSocketRecvBuf->addFreeBuf();
	}

	if(pstClientInfo->pSocketRecvBuf->getFreeSize() == 0)
	{
		//print error no buf size
		poWorker->close(iFd);
		snprintf(poWorker->m_szErrMsg,sizeof(poWorker->m_szErrMsg),"onTcpRead %s,%d,socket buf no memory",__FILE__,__LINE__);
		poWorker->onError(stSession,poWorker->m_szErrMsg,ERR_NO_BUFFER);
		return;

	}

	iSize = ::recv(iFd,pstClientInfo->pSocketRecvBuf->getFreeBuf(),pstClientInfo->pSocketRecvBuf->getFreeSize(),0);

	if(iSize > 0 ) 
	{
		pstClientInfo->pSocketRecvBuf->addData(iSize);

		int iWholePkgFlag = 0;

		int iRealPkgLen = 0;
		int iPkgLen = 0;

		if(pstClientInfo->pstServerInfo->pPackageFilter == NULL)
		{
			poWorker->close(iFd);
			snprintf(poWorker->m_szErrMsg,sizeof(poWorker->m_szErrMsg),"onTcpRead %s,%d,package filter is null",__FILE__,__LINE__);
			poWorker->onError(stSession,poWorker->m_szErrMsg,ERR_PKG_FILTER);
			return;
		}

		while ((iWholePkgFlag = pstClientInfo->pstServerInfo->pPackageFilter->isWholePkg(pstClientInfo->pSocketRecvBuf->getData(), pstClientInfo->pSocketRecvBuf->getSize(), iRealPkgLen, iPkgLen)) == 0 )
		{
			poWorker->onRead(stSession,pstClientInfo->pSocketRecvBuf->getData(),iPkgLen);
			if(poWorker->isClose(iFd))
				break;
			pstClientInfo->pSocketRecvBuf->removeData(iPkgLen);
		}


		if ( -2 == iWholePkgFlag )//非法数据包
		{
			//CCommMgr::getInstance().close(iFd);//非法数据包时，不关闭连接，而是重置接收缓冲区，是否关闭连接让上层处理
			pstClientInfo->pSocketRecvBuf->reset();
			snprintf(poWorker->m_szErrMsg,sizeof(poWorker->m_szErrMsg),"onTcpRead %s,%d,package invalid",__FILE__,__LINE__);
			poWorker->onError(stSession,poWorker->m_szErrMsg,ERR_INVALID_PACKAGE);
		}
	}
	else if(iSize == 0)
	{
		poWorker->onClose(stSession);
		poWorker->close(iFd);
	}
	else
	{
		if( errno == 104) //Connection reset by peer
		{
			poWorker->onClose(stSession);
			poWorker->close(iFd);
		}
		else if(errno == EAGAIN || errno == EINTR) //处理连接正常，IO不正常情况，不关闭连接
		{
			snprintf(poWorker->m_szErrMsg,sizeof(poWorker->m_szErrMsg),"onTcpRead EAGAIN or EINTR %s,%d,errno=%d,msg=%s",__FILE__,__LINE__,errno,strerror(errno));
			poWorker->onError(stSession,poWorker->m_szErrMsg,ERR_NOT_READY);
		}
		else
		{
			snprintf(poWorker->m_szErrMsg,sizeof(poWorker->m_szErrMsg),"onTcpRead %s,%d,errno=%d,msg=%s",__FILE__,__LINE__,errno,strerror(errno));
			poWorker->close(iFd);
			poWorker->onError(stSession,poWorker->m_szErrMsg,ERR_SOCKET);
		}

	}
}

int CNetWorker::close(const SSession & stSession)
{
	return close(stSession.iFd);
}

int CNetWorker::close(int iFd)
{
	if( m_vecClients[iFd] != NULL)
	{
		delete m_vecClients[iFd];
		m_vecClients[iFd] = NULL;
		m_oEvent.delFdEvent(iFd,CEvent::EV_READ|CEvent::EV_WRITE);
		return lce::close(iFd);
	}
	return 0;
}

int CNetWorker::write(const SSession &stSession,const char* pszData, const int iSize,bool bClose)
{

	if(isClose(stSession.iFd))
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,write error maybe client have closed",__FILE__,__LINE__);
		return -1;
	}

	SClientInfo * pstClientInfo = m_vecClients[stSession.iFd];

	SServerInfo * pstServerInfo = pstClientInfo->pstServerInfo;


	pstClientInfo->bNeedClose = bClose;
	int iSendBufSize = 0 ;
	int iSendSize = 0;

	if(pstClientInfo->pSocketSendBuf == NULL || pstClientInfo->pSocketSendBuf->getSize() == 0)
	{
		iSendSize=lce::send(stSession.iFd,pszData,iSize);

		if(iSendSize > 0 )
		{
			if (iSendSize < iSize)
			{
				if(pstClientInfo->pSocketSendBuf == NULL)
				{
					pstClientInfo->pSocketSendBuf=new CSocketBuf(pstServerInfo->dwInitSendBufLen,pstServerInfo->dwMaxSendBufLen);
				}
				if(!pstClientInfo->pSocketSendBuf->addData(pszData+iSendSize,iSize-iSendSize))
				{
					snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,write error buffer less than data",__FILE__,__LINE__);
					return -1;
				}
				m_oEvent.addFdEvent(stSession.iFd,CEvent::EV_WRITE,CNetWorker::onWrite,this);
			}
			else
			{
				if(bClose)
				{
					close(stSession.iFd);
				}
				else
				{
					m_oEvent.delFdEvent(stSession.iFd,CEvent::EV_WRITE);
				}
			}
		}
		else
		{
			if( errno == EAGAIN ||errno == EINTR  )
			{
				if(pstClientInfo->pSocketSendBuf == NULL)
				{
					pstClientInfo->pSocketSendBuf=new CSocketBuf(pstServerInfo->dwInitSendBufLen,pstServerInfo->dwMaxSendBufLen);
				}
				if(!pstClientInfo->pSocketSendBuf->addData(pszData,iSize))
				{
					snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,write error buffer less than data",__FILE__,__LINE__);
					return -1;
				}
				m_oEvent.addFdEvent(stSession.iFd,CEvent::EV_WRITE,CNetWorker::onWrite,this);
			}
			else
			{
				snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
				close(stSession.iFd);
				return -1;
			}
		}

	}
	else if((iSendBufSize = pstClientInfo->pSocketSendBuf->getSize()) > 0)
	{

		int iSendSize=lce::send(stSession.iFd,pstClientInfo->pSocketSendBuf->getData(),iSendBufSize);
		if (iSendSize > 0 )
		{
			pstClientInfo->pSocketSendBuf->removeData(iSendSize);
			iSendBufSize -= iSendSize;
		}
		else
		{
			if (errno != EAGAIN && errno != EINTR )
			{
				snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
				close(stSession.iFd);
				return -1;
			}
		}

		if( iSendBufSize > 0)
		{
			if(!pstClientInfo->pSocketSendBuf->addData(pszData,iSize))
			{
				snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,write error buffer less than data",__FILE__,__LINE__);
				return -1;
			}

			m_oEvent.addFdEvent(stSession.iFd,CEvent::EV_WRITE,CNetWorker::onWrite,this);
		}
		else
		{
			int iSendSize=lce::send(stSession.iFd,pszData,iSize);
			if(iSendSize > 0 )
			{
				if (iSendSize < iSize)
				{
					if(!pstClientInfo->pSocketSendBuf->addData(pszData+iSendSize,iSize-iSendSize))
					{
						snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,write error buffer less than data",__FILE__,__LINE__);
						return -1;
					}
					m_oEvent.addFdEvent(stSession.iFd,CEvent::EV_WRITE,CNetWorker::onWrite,this);
				}
				else
				{
					if(bClose)
					{
						close(stSession.iFd);
					}
					else
					{
						m_oEvent.delFdEvent(stSession.iFd,CEvent::EV_WRITE);
					}
				}
			}
			else
			{
				if( errno == EAGAIN ||errno == EINTR )
				{
					if(!pstClientInfo->pSocketSendBuf->addData(pszData,iSize))
					{
						snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,write error buffer less than data",__FILE__,__LINE__);
						return -1;
					}

					m_oEvent.addFdEvent(stSession.iFd,CEvent::EV_WRITE,CNetWorker::onWrite,this);
				}
				else
				{
					snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
					close(stSession.iFd);
					return -1;
				}
			}
		}
	}
	return 0;
}

int CNetWorker::write(int iFd)
{

	SClientInfo * pstClientInfo = m_vecClients[iFd];
	SServerInfo * pstServerInfo = pstClientInfo->pstServerInfo;

	SSession stSession;
	stSession.ddwBeginTime = lce::getTickCount();
	stSession.stClientAddr = pstClientInfo->stClientAddr;
	stSession.iFd = iFd;
	stSession.iSvrId = pstServerInfo->iSrvId;


	int iSendBufSize=pstClientInfo->pSocketSendBuf->getSize();

	if(iSendBufSize > 0)
	{
		int iSendSize=lce::send(iFd,pstClientInfo->pSocketSendBuf->getData(),iSendBufSize);
		if (iSendSize > 0 )
		{
			pstClientInfo->pSocketSendBuf->removeData(iSendSize);
			iSendBufSize -= iSendSize;
		}
		else
		{
			if (errno != EAGAIN && errno != EINTR )
			{
				snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
				close(iFd);
				onError(stSession,m_szErrMsg,ERR_SOCKET);
				return -1;
			}
		}
	}

	if( iSendBufSize == 0)
	{
		if (pstClientInfo->bNeedClose)
			close(pstClientInfo->iFd);
		else
			m_oEvent.delFdEvent(iFd,CEvent::EV_WRITE);
	}
	return 0;

}

void CNetWorker::onWrite(int iFd,void *pData)
{

	CNetWorker *poWorker = (CNetWorker*)pData;

	if(poWorker->isClose(iFd))
	{
		return;
	}
	poWorker->write(iFd);
}


};