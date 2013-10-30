#ifndef __NCE_NETWORKER_H
#define __NCE_NETWORKER_H

#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include "Utils.h"
#include "CSocketBuf.h"
#include "CPackageFilter.h"
#include "CH2T3PackageFilter.h"
#include "CH2ShortT3PackageFilter.h"
#include "CHttpPackageFilter.h"
#include "CRawPackageFilter.h"
#include "CEvent.h"
#include "signal.h"
#include "CThread.h"

using namespace std;

namespace lce
{
	static const float FD_TIMES =  1.2;

	enum APP_TYPE
	{
		SRV_TCP=1,
		SRV_UDP=2,
		CONN_TCP=3,
	};

	enum PKG_TYPE
	{
		PKG_EXT = 0,
		PKG_H2ST3 = 1,
		PKG_H2LT3 = 2,
		PKG_HTTP = 3,
		PKG_RAW = 4,
	};

	enum ERR_TYPE
	{
		ERR_SOCKET = 1,
		ERR_INVALID_PACKAGE = 2,
		ERR_MAX_CLIENT = 3,
		ERR_NO_BUFFER = 4,
		ERR_NOT_READY = 5,
		ERR_SYSTEM = 6,
		ERR_PKG_FILTER = 7,
	};

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

	struct SServerInfo
	{
		SServerInfo()
		{
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
		uint32_t dwInitRecvBufLen;
		uint32_t dwMaxRecvBufLen;
		uint32_t dwInitSendBufLen;
		uint32_t dwMaxSendBufLen;
		CPackageFilter *pPackageFilter;
		int iType;

		~SServerInfo()
		{
			if(pPackageFilter != NULL && iPkgType != PKG_EXT) { delete pPackageFilter;pPackageFilter = NULL ;}
		}
	};

	class CNetWorker;

	struct SClientInfo
	{
		SClientInfo(){ memset(this,0,sizeof(SClientInfo));}
		int iFd;
		bool bNeedClose;
		CNetWorker *poWorker;
		SServerInfo *pstServerInfo;
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

		int init(uint32_t dwMaxClient = 10000);

		virtual void onRead(SSession &stSession,const char * pszData, const int iSize){
			throw std::runtime_error("not implement onRead");
		}
		virtual void onClose(SSession &stSession){
			throw std::runtime_error("not implement onColse");
		}
		virtual void onConnect(SSession &stSession,bool bOk){
			throw std::runtime_error("not implement onConnect");
		}
		virtual void onError(SSession &stSession,const char * szErrMsg,int iError){
			throw std::runtime_error("not implement  onError");
		}

		int watch(int iFd,void *pData);
		int close(const SSession &stSession);
		int write(const SSession &stSession,const char* pszData, const int iSize,bool bClose = true);

		int createAsyncConn(int iPkgType = PKG_RAW,uint32_t dwInitRecvBufLen =10240,uint32_t dwMaxRecvBufLen=102400,uint32_t dwInitSendBufLen=102400,uint32_t dwMaxSendBufLen=1024000);

		int connect(int iSrvId,const string &sIp,uint16_t wPort);

	private:
		int run();

		inline bool isClose(int iFd)
		{
			return (m_vecClients[iFd] == NULL);
		}
		int close(int iFd);
		int write(int iFd);

		static void onWrite(int iFd,void *pData);
		static void onTcpRead(int iFd,void *pData);
		static void onConnect(int iFd,void *pData);
	private:
		CEvent m_oEvent;
		vector <SClientInfo *> m_vecClients;
		vector <SServerInfo *> m_vecServers;

		char m_szErrMsg[1024];
		

	};
};

#endif