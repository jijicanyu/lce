#include <iostream>
#include <CNetWorkerMgr.h>

using namespace lce;


class CWorker:public CNetWorker
{
public:
	void onRead(SSession &stSession,const char * pszData, const int iSize)
	{
		//cout<<"id="<<getId()<<",fd="<<stSession.iFd<<"onRead"<<endl;
		string sData;
		sData+="HTTP/1.1 200 OK\r\n";
		sData+="Connection: Close\r\n";
		sData+="Content-Length: 5\r\n\r\n";
		sData+="hello";

		write(stSession,(char*)sData.data(),sData.size());
	}
	void onClose(SSession &stSession)
	{
		cout<<"id="<<getId()<<",fd="<<stSession.iFd<<"onClose"<<endl;
	}
	void onConnect(SSession &stSession,bool bOk)
	{
		//cout<<"id="<<getId()<<",fd="<<stSession.iFd<<"onConnect"<<endl;
	}

};

template<>	CNetWorkerMgr<CWorker>* CNetWorkerMgr<CWorker>::m_pInstance = NULL;


int main(int argc,char **argv)
{
 

	CNetWorkerMgr<CWorker>::getInstance().init("0.0.0.0",8002,3,10000);
	cout<<CNetWorkerMgr<CWorker>::getInstance().getErrMsg()<<endl;
	CNetWorkerMgr<CWorker>::getInstance().start();

	return 0;
}