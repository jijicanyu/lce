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

	if( init() < 0)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,init worker error",__FILE__,__LINE__);
		return -1;
	}
	
	return 0;
}

int CNetWorker::createAsyncConn(int iPkgType /* = PKG_RAW */,uint32_t dwInitRecvBufLen /* =10240 */,uint32_t dwMaxRecvBufLen/* =102400 */,uint32_t dwInitSendBufLen/* =102400 */,uint32_t dwMaxSendBufLen/* =1024000 */)
{
	SServerInfo *pstServerInfo = new SServerInfo;

	pstServerInfo->sIp ="";
	pstServerInfo->wPort = 0;

	pstServerInfo->iType = CONN_TCP;


	pstServerInfo->dwInitRecvBufLen=dwInitRecvBufLen;
	pstServerInfo->dwMaxRecvBufLen=dwMaxRecvBufLen;

	pstServerInfo->dwInitSendBufLen=dwInitSendBufLen;
	pstServerInfo->dwMaxSendBufLen=dwMaxSendBufLen;

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

	pstServerInfo->iSrvId = START_SRV_ID + m_mapServers.size();
	pstServerInfo->iFd=0;
	m_mapServers[pstServerInfo->iSrvId] = pstServerInfo;

	return pstServerInfo->iSrvId;
}


int CNetWorker::setPkgFilter(int iSrvId,CPackageFilter *pPkgFilter)
{
	if(pPkgFilter == NULL)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,pkgfilter pointer is null",__FILE__,__LINE__);
		return -1;
	}

	map<uint32_t,SServerInfo *>::iterator it = m_mapServers.find(iSrvId);

	if(it == m_mapServers.end())
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,iSrvId error",__FILE__,__LINE__);
		return -1;
	}

	it->second->pPackageFilter = pPkgFilter;

	return 0;
}


int CNetWorker::connect(int iSrvId,const string &sIp,uint16_t wPort,void *pData)
{

	map<uint32_t,SServerInfo *>::iterator it = m_mapServers.find(iSrvId);

	if(it == m_mapServers.end())
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,iSrvId error",__FILE__,__LINE__);
		return -1;
	}

	SServerInfo *pstServerInfo = it->second;

	if(pstServerInfo->iType == CONN_TCP)
	{

		SClientInfo * pstClientInfo = new SClientInfo;
		pstClientInfo->iFd=lce::createTcpSock();
		pstClientInfo->pstServerInfo = pstServerInfo;
		pstClientInfo->pData = pData;

		if (pstClientInfo->iFd < 0)
		{
			snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
			delete pstClientInfo;
			pstClientInfo = NULL;
			return -1;

		}

		lce::setReUseAddr(pstClientInfo->iFd);
		lce::setNBlock(pstClientInfo->iFd);

		pstClientInfo->stClientAddr.sin_family = AF_INET;
		pstClientInfo->stClientAddr.sin_port = htons(wPort);
		pstClientInfo->stClientAddr.sin_addr.s_addr = inet_addr(sIp.c_str());
		memset(&(pstClientInfo->stClientAddr.sin_zero),0,8);

		int iRet = lce::connect(pstClientInfo->iFd,sIp,wPort);

		if(iRet != -1)
		{
			SSession stSession;
			stSession.ddwBeginTime=lce::getTickCount();
			stSession.iFd=pstClientInfo->iFd;
			stSession.iSvrId=pstServerInfo->iSrvId;
			stSession.stClientAddr=pstClientInfo->stClientAddr;
			m_vecClients[pstClientInfo->iFd] = pstClientInfo;
			onConnect(stSession,true,pData);

			if(isClose(pstClientInfo->iFd))
			{
				snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,connect error maybe client have closed",__FILE__,__LINE__);
				return -1;
			}
			return pstClientInfo->iFd;
		}
		else
		{

			if (errno == EINPROGRESS)
			{
				m_oEvent.addFdEvent(pstClientInfo->iFd,CEvent::EV_WRITE,CNetWorker::onConnect,this);
				m_vecClients[pstClientInfo->iFd]=pstClientInfo;
				return pstClientInfo->iFd;
			}
			else
			{
				snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,errno:%d,error:%s",__FILE__,__LINE__,errno,strerror(errno));
				delete pstClientInfo;
				pstClientInfo = NULL;
				return -1;
			}
		}
		return -1;
	}
	else
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s,%d,server iType error",__FILE__,__LINE__);
		return -1;
	}
	return -1;
}

void CNetWorker::onConnect(int iFd,void *pData)
{

	CNetWorker *poWorker = (CNetWorker*)pData;

	if(poWorker->isClose(iFd))
	{
		return;
	}

	SClientInfo *pstClientInfo = poWorker->m_vecClients[iFd];

	poWorker->m_oEvent.delFdEvent(iFd,CEvent::EV_WRITE);

	SServerInfo * pstServerInfo = pstClientInfo->pstServerInfo;

	int error;
	socklen_t ilen = sizeof(int);
	getsockopt(iFd, SOL_SOCKET, SO_ERROR, &error, &ilen);


	SSession stSession;
	stSession.ddwBeginTime = lce::getTickCount();
	stSession.iFd = iFd;
	stSession.iSvrId = pstServerInfo->iSrvId;
	stSession.stClientAddr = pstClientInfo->stClientAddr;

	if(error == 0)
	{
		poWorker->onConnect(stSession,true,pstClientInfo->pData);
		
		if(!poWorker->isClose(iFd))
		{
			pstClientInfo->pData = poWorker;
			poWorker->m_oEvent.addFdEvent(pstClientInfo->iFd,CEvent::EV_READ,CNetWorker::onTcpRead,pstClientInfo);
		}
	}
	else
	{
		poWorker->onConnect(stSession,false,pstClientInfo->pData);
		poWorker->close(iFd);
	}
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
	CNetWorker *poWorker = (CNetWorker*)pstClientInfo->pData;

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


int CNetWorker::addTimer(int iTimerId,uint32_t dwExpire,void *pData)
{	
	m_mapTimeProcs[iTimerId].pData = pData;
	return m_oEvent.addTimer(iTimerId,dwExpire,CNetWorker::onTimerProc,this);
}

int CNetWorker::delTimer(int iTimerId)
{
	m_mapTimeProcs.erase(iTimerId);
	return m_oEvent.delTimer(iTimerId);
}

void CNetWorker::onTimerProc(int iTimerId,void *pData)
{
	CNetWorker *poWorker = (CNetWorker*)pData;

	MAP_TIMER_PROC::iterator it = poWorker->m_mapTimeProcs.find(iTimerId);

	if(it != poWorker->m_mapTimeProcs.end())
	{
		void *pClientData = it->second.pData;
		poWorker->m_mapTimeProcs.erase(iTimerId);

		poWorker->onTimer(iTimerId,pClientData);
	}

}


};