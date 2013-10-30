#ifndef __NCE_LRUCACHE_H__
#define __NCE_LRUCACHE_H__

#include<iostream>
#include<map>
#include <stdint.h>
#include <sys/time.h>
#include <pthread.h>
#include<tr1/unordered_map>

using namespace std;
namespace lce
{
class CLruCache
{
private:

	struct SValue
	{
		uint32_t dwCTime;
		uint32_t dwExpireTime;
		uint64_t ddwAccessTime;
		string sValue;

	};

public:

	typedef tr1::unordered_map<string,SValue> MAP_CACHE;
	typedef map<uint64_t,string> MAP_TIME_KEY;

	CLruCache()
	{ 
		m_dwSize = 10000; 
		::pthread_mutex_init(&m_sect, NULL);
	}

	~CLruCache()
	{
		::pthread_mutex_destroy(&m_sect);
	}

	bool init(size_t dwSize = 10000)
	{
		m_dwSize = dwSize;
		return true;
	}

	bool set(const string &sKey,const string &sValue,uint32_t dwExpireTime = 0)
	{
		::pthread_mutex_lock(&m_sect);
		if(m_mapCache.size() > m_dwSize)
		{
			int i = 0;
			for(MAP_TIME_KEY::iterator it = m_mapTimeKey.begin();it!=m_mapTimeKey.end();)
			{
				m_mapCache.erase(it->second);
				m_mapTimeKey.erase(it++);
				i++;
				if( i > 50) break;
			}
		}
		uint64_t ddwTime = getTickCount();
		SValue stValue;
		stValue.dwCTime = time(0);
		stValue.dwExpireTime = dwExpireTime;
		stValue.sValue = sValue;
		stValue.ddwAccessTime = ddwTime;

		MAP_CACHE::iterator it =  m_mapCache.find(sKey);

		if(it!=m_mapCache.end())
		{
			m_mapTimeKey.erase(it->second.ddwAccessTime);
		}

		m_mapCache[sKey] = stValue;
		m_mapTimeKey[ddwTime] = sKey;

		::pthread_mutex_unlock(&m_sect);
		return true;
	}

	bool get(const string &sKey,string &sValue)
	{
		::pthread_mutex_lock(&m_sect); 
		MAP_CACHE::iterator it = m_mapCache.find(sKey);
		if( it != m_mapCache.end())
		{
			if (it->second.dwExpireTime != 0)
			{
				uint32_t dwNow = time(0);
				if(dwNow - it->second.dwCTime > it->second.dwExpireTime)
				{
					m_mapTimeKey.erase(it->second.ddwAccessTime);
					m_mapCache.erase(sKey);
					::pthread_mutex_unlock(&m_sect);
					return false;
				}
			}

			sValue = it->second.sValue;
			uint64_t ddwOldTime =  it->second.ddwAccessTime;
			it->second.ddwAccessTime = getTickCount();

			MAP_TIME_KEY::iterator it2 = m_mapTimeKey.find(ddwOldTime);

			if(it2!=m_mapTimeKey.end())
			{
				m_mapTimeKey.erase(it2);
			}
			m_mapTimeKey[it->second.ddwAccessTime] = sKey;
			::pthread_mutex_unlock(&m_sect);

			return true;
		}

		::pthread_mutex_unlock(&m_sect);
		return false;
	}

	void clear()
	{
		::pthread_mutex_lock(&m_sect);
		m_mapCache.clear();
		m_mapTimeKey.clear();
		::pthread_mutex_unlock(&m_sect);
	}


	bool del(const string &sKey)
	{
		::pthread_mutex_lock(&m_sect);
		MAP_CACHE::iterator it = m_mapCache.find(sKey);
		if(it!= m_mapCache.end())
		{
			m_mapTimeKey.erase(it->second.ddwAccessTime);
			m_mapCache.erase(it);
		}
		::pthread_mutex_unlock(&m_sect);
		return true;
	}

public:
	inline uint64_t getTickCount()
	{
		timeval tv;
		gettimeofday(&tv, 0);
		return tv.tv_sec * 1000000 + tv.tv_usec;
	}
private:

	::pthread_mutex_t m_sect;

	MAP_CACHE m_mapCache;
	MAP_TIME_KEY m_mapTimeKey;
	size_t m_dwSize;
};
};

#endif