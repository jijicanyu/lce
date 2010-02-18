#include <iostream>
#include <CNetWorkerMgr.h>
#include <CSingleton.h>
using namespace lce;
 

class CWorker:public CNetWorker
{
public:
	int init()
	{
		addTimer(0,1000,NULL);
		m_iConnHandler = createAsyncConn(PKG_HTTP);
		cout<<m_iConnHandler<<endl;
		return 0;
	}
	void onRead(SSession &stSession,const char * pszData, const int iSize)
	{

		if(stSession.iSvrId >= START_SRV_ID)
		{
			cout<<"id="<<getId()<<",fd="<<stSession.iFd<<",onRead"<<endl;

			string sRecv;
			sRecv.assign(pszData,iSize);
			cout<<sRecv;
		}
		else
		{
			string sData;
			sData+="HTTP/1.1 200 OK\r\n";
			sData+="Connection: Close\r\n";
			sData+="Content-Length: 5\r\n\r\n";  
			sData+="hello";

			write(stSession,(char*)sData.data(),sData.size());
		}

	}
	void onClose(SSession &stSession)
	{
		cout<<"id="<<getId()<<",fd="<<stSession.iFd<<",onClose"<<endl;
	}
	void onConnect(SSession &stSession,bool bOk,void *pData)
	{
		cout<<"id="<<getId()<<",fd="<<stSession.iFd<<",onConnect"<<endl;
		string sReq = "GET / HTTP/1.1\r\n\r\n";
		write(stSession,sReq.data(),sReq.size(),false);
	}

	void onError(SSession &stSession,const char * szErrMsg,int iError)
	{
		cout<<"onError"<<szErrMsg<<endl;
	}

	void onTimer(int iTimeId,void *pData)
	{
		cout<<"onTimer"<<endl;

		connect(m_iConnHandler,"61.135.169.105",80);
		//addTimer(0,1000,NULL);

	}

private:
	int m_iConnHandler;
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