#ifndef __NCE_CPROCESSOR_H
#define __NCE_CPROCESSOR_H

namespace lce
{

struct SSession;

class CProcessor
{
public:
	virtual bool onRead(SSession &stSession,const char * pszData, const int iSize){return true;}
	virtual void onClose(SSession &stSession){}
	virtual void onConnect(SSession &stSession,bool bOk){}
	virtual void onError(char * szErrMsg){}
	virtual void onTimer(uint32_t dwTimeId,void *pData){}
	virtual void onMessage(uint32_t dwMsgType,void *pData){}
	virtual void onSignal(int iSignal){ exit(0); }
};

};

#endif