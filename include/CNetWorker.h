#ifndef __NCE_NETWORKER_H
#define __NCE_NETWORKER_H

#include <string>
#include <vector>
#include <iostream>
#include <sys/eventfd.h>
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
#include "CLFQueue.h"

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
		void * pData;
		struct sockaddr_in stClientAddr;
		uint64_t ddwBeginTime;
		int64_t getDelayTime()
		{
			uint64_t ddwCurTime = lce::getTickCount();
			return ddwCurTime - ddwBeginTime;
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

	class CNetWorker :public CThread
	{
	public:

		int init();

		virtual void onRead(SSession &stSession,const char * pszData, const int iSize){
			throw std::runtime_error("not implement onRead");
		}
		virtual void onClose(SSession &stSession){
			throw std::runtime_error("not implement onColse");
		}
		virtual void onConnect(SSession &stSession,bool bOk){
			throw std::runtime_error("not implement onConnect");
		}

		int watch(int iFd,void *pData);

		int write(const SSession &stSession,char*pszData,int iSize,bool bClose = true);
	private:
		int run();
		static void onWrite(int iFd,void *pData);
		static void onTcpRead(int iFd,void *pData);
		static void onConnect(int iFd,void *pData);
		static void onQueueEvent(int iFd,void *pData);
	private:
		CEvent m_oEvent;
		vector <SClientInfo *> m_vecClients;
		int m_iEventFd;
		CLFQueue<SClientInfo *> m_queClients;
	};
};

#endif