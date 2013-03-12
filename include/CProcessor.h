#ifndef __NCE_CPROCESSOR_H
#define __NCE_CPROCESSOR_H

namespace lce
{

struct SSession;

class CProcessor
{
public:
	virtual void onRead(SSession &stSession,const char * pszData, const int iSize){}
	virtual void onClose(SSession &stSession){}
	virtual void onConnect(SSession &stSession,bool bOk){}
	virtual void onError(SSession &stSession,const char * szErrMsg,int iError){}
	virtual void onTimer(uint32_t dwTimeId,void *pData){}
	virtual void onMessage(int dwMsgType,void *pData){}
	virtual void onSignal(int iSignal){}
};

};

#endif