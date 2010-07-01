#include "CAsyncLog.h"


namespace lce
{


CAsyncLog::~CAsyncLog()
{
	if (m_ofsOutFile.is_open())
	{
		m_ofsOutFile.close();
	}

	if(m_poWriteBuffer1 != NULL) delete m_poWriteBuffer1;
	if(m_poWriteBuffer2 != NULL) delete m_poWriteBuffer2; 
}

bool CAsyncLog::init(const std::string &sLogFilePath,int iLogSecs, const unsigned long dwLogFileMaxSize, const unsigned int uiLogFileNum, const bool bShowCmd)
{

	m_sCurDate = getDate();
	m_sLogFilePath=sLogFilePath+"_"+m_sCurDate+".log",
	m_sLogBasePath = sLogFilePath;
	m_dwLogFileMaxSize = dwLogFileMaxSize;
	m_uiLogFileNum = uiLogFileNum;
	m_bShowCmd = bShowCmd;

	m_poWriteBuffer1 = new std::string;
	m_poWriteBuffer2 = new std::string;
	m_poCurWirteBuffer = m_poWriteBuffer1;
	m_iLogSecs = iLogSecs;
	m_iCurWriteBufferFlag = 1;

	start();

	return true;
}

bool CAsyncLog::shiftFiles()
{

	if (m_sCurDate != getDate())
	{
		m_sLogFilePath=m_sLogBasePath+"_"+getDate()+".log";
		m_sCurDate = getDate();
		if (m_ofsOutFile.is_open())
		{
			m_ofsOutFile.close();
		}
	}

	size_t dwFileSize=this->getFileSize(m_sLogFilePath);

	if (dwFileSize>=m_dwLogFileMaxSize)
	{
		if (m_ofsOutFile.is_open())
		{
			m_ofsOutFile.close();
		}

		char szLogFileName[1024];
		char szNewLogFileName[1024];

		snprintf(szLogFileName,sizeof(szLogFileName),"%s_%s_%u.log",m_sLogBasePath.c_str(),getDate().c_str(),m_uiLogFileNum-1);
		if (access(szLogFileName, F_OK) == 0)
		{
			if (remove(szLogFileName) < 0 )
			{
				snprintf(m_szErrMsg, sizeof(m_szErrMsg),"remove error: errno=%d.",errno);
				return false;
			}
		}

		for(int i=m_uiLogFileNum-2; i>=0; i--)
		{
			if (i == 0)
				snprintf(szLogFileName,sizeof(szLogFileName),"%s_%s.log",m_sLogBasePath.c_str(),getDate().c_str());
			else
				snprintf(szLogFileName,sizeof(szLogFileName),"%s_%s_%u.log",m_sLogBasePath.c_str(),getDate().c_str(),i);

			if (access(szLogFileName, F_OK) == 0)
			{
				snprintf(szNewLogFileName,sizeof(szNewLogFileName),"%s_%s_%d.log",m_sLogBasePath.c_str(),getDate().c_str(),i+1);
				if (rename(szLogFileName,szNewLogFileName) < 0 )
				{
					snprintf(m_szErrMsg, sizeof(m_szErrMsg),"rename error:errno=%d.",errno);
					return false;
				}
			}
		}
	}

	return true;
}

bool CAsyncLog::writeFile(const std::string &str, const bool bEnd)
{

	if (!shiftFiles())
	{
		return false;
	}
	

	if (!m_ofsOutFile.is_open())
	{
		m_ofsOutFile.open(m_sLogFilePath.c_str(),std::ios::out|std::ios::app);
	}

	if(m_ofsOutFile.fail())
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"open file<%s> err!", m_sLogFilePath.c_str());
		return false;
	}

	m_ofsOutFile << str;
	
	if (bEnd)
	{
		m_ofsOutFile << std::endl;
	}

	//m_ofsOutFile.flush();

	return true;
}


int CAsyncLog::run()
{
	int iMillSecs = 0;
	while(true)
	{
		usleep(100000);
		
		if((iMillSecs >= m_iLogSecs*1000 && m_poCurWirteBuffer->size()>0) ||m_poCurWirteBuffer->size() > 1024*1024*2)
		{

			if(m_iCurWriteBufferFlag == 1)
			{
				if(m_poWriteBuffer2 == NULL)
					m_poWriteBuffer2 = new std::string;

				m_mutex.lock();
				m_poCurWirteBuffer = m_poWriteBuffer2;
				m_mutex.unlock();

				writeFile(*m_poWriteBuffer1,false);
				delete m_poWriteBuffer1;
				m_poWriteBuffer1 = NULL;

				m_iCurWriteBufferFlag = 2;
			}
			else if(m_iCurWriteBufferFlag == 2)
			{
				if(m_poWriteBuffer1 == NULL)
					m_poWriteBuffer1 = new std::string;

				m_mutex.lock();
				m_poCurWirteBuffer = m_poWriteBuffer1;
				m_mutex.unlock();
				writeFile(*m_poWriteBuffer2,false);

				delete m_poWriteBuffer2;
				m_poWriteBuffer2 = NULL;

				m_iCurWriteBufferFlag = 1;
			}
			
			iMillSecs = 0;
		}
		iMillSecs += 100;

	}
	return 0;
}



bool CAsyncLog::writeBuffer(const std::string &str, const bool bEnd)
{

	lce::CAutoLock autoLock(m_mutex);

	if (m_bShowCmd)
	{
		std::cout << str << std::endl;
	}

	m_poCurWirteBuffer->append(str);

	if (bEnd)
	{
		m_poCurWirteBuffer->append("\n");
	}

	return true;
}

bool CAsyncLog::write(const char *sFormat, ...)
{

	char szTemp[10240];
	va_list ap;

	va_start(ap, sFormat);
	vsnprintf(szTemp,sizeof(szTemp),sFormat, ap);
	va_end(ap);
	std::string sLog;
	sLog = "[" + getDateTime() + "]:";
	sLog += szTemp;

	if(!writeBuffer(sLog))
		return false;


	return true;
}

bool CAsyncLog::write(const std::string& sMsg)
{

	if(!writeBuffer("[" + getDateTime() + "]" + sMsg))
		return false;

	return true;
}

bool CAsyncLog::writeRaw(const std::string& sMsg)
{

	if(!writeBuffer(sMsg))
		return false;

	return true;
}


};
