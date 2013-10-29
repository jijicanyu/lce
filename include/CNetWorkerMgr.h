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

template <class T>

class CNetWorkerMgr
{
public:
	init(const string& sIp,const uint16_t wPort,uint32_t dwThreadNum,uint32_t dwMaxClient);
	static CNetWorkerMgr & getInstance()
	{

		if (NULL == m_pInstance)
		{
			m_pInstance = new CNetWorkerMgr;
		}
		return *m_pInstance;
	}
private:
    static void onAccept(int iFd,void *pData);


private:
	static CNetWorkerMgr *m_pInstance;

	CEvent m_oEvent;
	char m_szErrMsg[1024];
	uint32_t m_dwMaxClient;
	uint32_t m_dwClientNum;
	vector<CNetWorker*> vecWorkers;
};

};

#endif