#include <iostream>
#include <CNetWorkerMgr.h>

using namespace lce;
 

class CWorker:public CNetWorker
{
public:
	void onRead(SSession &stSession,const char * pszData, const int iSize)
	{
		//cout<<"id="<<getId()<<",fd="<<stSession.iFd<<",onRead"<<endl;
		string sData;
		sData+="HTTP/1.1 200 OK\r\n";
		sData+="Connection: Close\r\n";
		sData+="Content-Length: 5\r\n\r\n";  
		sData+="hello";

		write(stSession,(char*)sData.data(),sData.size());
	}
	void onClose(SSession &stSession)
	{
		cout<<"id="<<getId()<<",fd="<<stSession.iFd<<",onClose"<<endl;
	}
	void onConnect(SSession &stSession,bool bOk)
	{
		//cout<<"id="<<getId()<<",fd="<<stSession.iFd<<",onConnect"<<endl;
	}

	void onError(SSession &stSession,const char * szErrMsg,int iError)
	{
		cout<<"onError"<<szErrMsg<<endl;
	}
};


int main(int argc,char **argv)
{
 
	CNetWorkerMgr<CWorker> oNetWorkerMgr;
		
	oNetWorkerMgr.init(2,10000);
	
	int iSrv1 = oNetWorkerMgr.createSrv("0.0.0.0",8002,PKG_HTTP);

	cout<<oNetWorkerMgr.getErrMsg()<<endl;

	oNetWorkerMgr.start();

	return 0;
}