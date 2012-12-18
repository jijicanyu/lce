#include <iostream>
#include "../Utils.h"
#include "../CEvent.h"
#include "../CCommMgr.h"
#include "../CH2ShortT3PackageFilter.h"
#include "../CRawPackageFilter.h"
#include "../CPackage.h"
#include "../CHttpPackageFilter.h"
#include "../CHttpParser.h"
#include "../CHttpResponse.h"
#include "../CTask.h"
#include "../CProcessor.h"
#include "../CAnyValue.h"

using namespace std;
using namespace lce;

int iSrv1;

struct SRequest
{
    uint32_t dwReqId;
    SSession stSession;
    CHttpParser oParser;
    CHttpResponse oResponse;

};


#pragma pack(1)

struct SHead
{
public:
	void setStx(){ m_cStx = 0x2; }
	void setLen(uint32_t dwLen){ m_dwLen = htonl(dwLen); }
	void setCmd(uint16_t wCmd){m_wCmd = htons(wCmd);}
	void setSeq(uint32_t dwSeq){ m_dwSeq = htonl(dwSeq); }
	uint32_t getLen(){ return ntohl(m_dwLen); }
	uint16_t getCmd(){ return ntohs(m_wCmd); }
	uint32_t getSeq(){ return ntohl(m_dwSeq); }

private:
	uint8_t m_cStx;
	uint32_t m_dwLen;
	uint16_t m_wCmd;
	uint32_t m_dwSeq;
};

#pragma pack()

class CProCenter : public CTask ,public CProcessor
{
private:
    CProCenter(){ dwCount = 0;dwOutCount=0;dwOutCount2=0;}
    static CProCenter *m_pInstance;
	int dwCount;
	int dwOutCount;
	int dwOutCount2;
public:
	

    bool onRead(SSession &stSession,const char * pszData, const int iSize)
    {

		dwCount++;
        SRequest *pstRequest=new SRequest;

        pstRequest->stSession=stSession;
        pstRequest->oParser.setData(pszData,iSize);
		
		/*
        pstRequest->oResponse.begin();
        pstRequest->oResponse.setStatusCode(200);
        pstRequest->oResponse<<"Hello world";
        pstRequest->oResponse.end();
        CCommMgr::getInstance().write(pstRequest->stSession,pstRequest->oResponse.data(),pstRequest->oResponse.size(),true);
        delete pstRequest;
		*/
		
        if(CProCenter::getInstance().dispatch(100,pstRequest)< 0)
		{
			cout<<CProCenter::getInstance().getErrMsg()<<endl;
		}
		//CProCenter::getInstance().dispatch()


    }


    void onWork(int iTaskType,void *pData)
    {

		//cout<<"type="<<iTaskType<<endl;
        SRequest *pstRequest=(SRequest*)pData;

        pstRequest->oResponse.begin();
        pstRequest->oResponse.setStatusCode(200);
        pstRequest->oResponse<<"Hello world";
        pstRequest->oResponse.end();
        if(CCommMgr::getInstance().sendMessage(iTaskType,this,pstRequest)<0)
		{
			cout<<"end error"<<endl;
		}
    }



    void onMessage(uint32_t dwMsgType,void *pData)
    {
		dwOutCount++;
        SRequest *pstRequest=(SRequest*)pData;
        CCommMgr::getInstance().write(pstRequest->stSession,pstRequest->oResponse.data(),pstRequest->oResponse.size(),true);
        delete pstRequest;
    }


	void onClose(SSession &stSession)
	{
		printf("onclose id=%d\n",stSession.iFd);
		//cout<< "onClose"<<endl;

	}

	void onConnect(SSession &stSession,bool bOk)
	{
		if(bOk)
		{

			printf("onconnect id=%d ok\n",stSession.iFd);
			dwOutCount++;

			CAnyValuePackage<SHead> oPkg;
			oPkg["name"]="starjiang";
			oPkg["pwd"]="840206";
			oPkg["params"]["xxx"];

			oPkg.head().setStx();
			oPkg.head().setCmd(1001);
			oPkg.encodeJSON();
			oPkg.head().setLen(oPkg.size()+1);
			oPkg.setEtx();


			CCommMgr::getInstance().write(stSession,oPkg.data(),oPkg.size(),false);

		}
		else
		{
			printf("onconnect id=%d fail\n",stSession.iFd);
		}

	}

	void onError(char * szErrMsg)
	{
		cout<<szErrMsg<<endl;
	}

	void onTimer(uint32_t dwTimerId,void *pData)
	{
		if(dwTimerId == 0)
		{
			CCommMgr::getInstance().addTimer(dwTimerId,2000,this,pData);
			cout<<"dwCount="<<dwCount<<" dwOut="<<dwOutCount<<endl;
		}
		else if(dwTimerId == 1)
		{
			dwCount++;
			CCommMgr::getInstance().connect(iSrv1,"127.0.0.1",8080);
			CCommMgr::getInstance().addTimer(dwTimerId,1000,this,pData);
		}

	}

	
	void onSignal(int iSignal)
	{
		switch(iSignal)
		{
			case SIGINT:
			{
				cout<<"stopping..."<<endl;
				CCommMgr::getInstance().stop();
			}
			break;
			case SIGHUP:
			{
				cout<<"sighup"<<endl;
				exit(0);
			}
			break;
		}
	}
	
    static CProCenter &getInstance()
    {
        if (NULL == m_pInstance)
        {
			m_pInstance = new CProCenter;
		}
		return *m_pInstance;
    }


};


CProCenter *CProCenter::m_pInstance = NULL;



int main()
{
    //CH2ShortT3PackageFilter oCPackageFilter;
    //lce::initDaemon(); //后台运行

    CProCenter::getInstance().init(8,50000);
    CProCenter::getInstance().run();

    if(CCommMgr::getInstance().init() < 0)
    {
        printf("%s\n",CCommMgr::getInstance().getErrMsg());
        return 0;
    }

    CHttpPackageFilter oCHttpPackageFilter;

	iSrv1=CCommMgr::getInstance().createAsyncConn(1024*10,1024*100,1024*10,1024*100);
	
    if(iSrv1 < 0 )
    {
        cout<<CCommMgr::getInstance().getErrMsg()<<endl;
    }

	CCommMgr::getInstance().setProcessor(iSrv1,&CProCenter::getInstance(),&oCHttpPackageFilter);
	CCommMgr::getInstance().setMaxClients(100000);

    CCommMgr::getInstance().addTimer(0,2000,&CProCenter::getInstance(),NULL);
    CCommMgr::getInstance().addTimer(1,2000,&CProCenter::getInstance(),NULL);
    CCommMgr::getInstance().addSigHandler(SIGINT,&CProCenter::getInstance());

    CCommMgr::getInstance().start();
    return 0;
}
