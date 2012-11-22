#ifndef __NCE_SHM_HASH_MAP_H__
#define __NCE_SHM_HASH_MAP_H__

#include <utility>
#include <memory>
#include <cassert>
#include "CShm.h"
using std::pair;

namespace lce
{

	enum { __num_primes = 28 };
	static const unsigned long __prime_list[__num_primes] =
	{
		53ul,         97ul,         193ul,       389ul,       769ul,
		1543ul,       3079ul,       6151ul,      12289ul,     24593ul,
		49157ul,      98317ul,      196613ul,    393241ul,    786433ul,
		1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul,
		50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul, 
		1610612741ul, 3221225473ul, 4294967291ul
	};
	#define SHM_HASH_SIGN_START  "lce shm hash map head ver 1.0 start"
	#define SHM_HASH_SIGN_END  "lce shm hash map head ver 1.0 end"

	template<typename T>
	class CShmHashMap;


	struct SShmHashNodeBase{
		unsigned long dwHashNextOffset;	//hash列表使用(地址偏移量)SHashNodeBase*
		unsigned long dwHashPreOffset;	//hash列表使用(地址偏移量)SHashNodeBase*
		unsigned long dwListNextOffset;	//空闲或使用列表指针使用(地址偏移量)SHashNodeBase*
		unsigned long dwListPreOffset;	//空闲或使用列表指针使用(地址偏移量)SHashNodeBase*
	};

	template<typename T>
	struct SShmHashNode: public SShmHashNodeBase{
		SShmHashNode(){}
		//unsigned long dwKey;	//key
		//T  tVal;				//数据
		std::pair<const unsigned long, T > value;
		size_t dwText;	//test 数据
	};

	//iterator
	template<typename T>
	class CShmHashMapIterator
	{
	public:
		typedef CShmHashMapIterator<T> _self;
		typedef std::pair<const unsigned long, T > pair_value;
	public:
		inline CShmHashMapIterator();
		inline ~CShmHashMapIterator();
		inline CShmHashMapIterator(const unsigned long dwValueOffset, void* pStartAddr);
		inline _self& operator=(const _self& rhs);
		//inline _self& operator=(SShmHashNodeBase* pValue);
		inline _self& Assign(const unsigned long dwValueOffset, void* pStartAddr);
		inline CShmHashMapIterator(const _self& rhs);
		inline const _self& operator++() const;
		inline _self& operator++() ;
		inline _self operator++(int);
		inline const _self operator++(int) const;
		inline _self& operator--();
		inline _self operator--(int);
		inline pair_value& operator*();
		inline pair_value* operator&();
		inline const pair_value* operator->() const;
		inline pair_value* operator->();
		bool operator==(const _self& it) const;
		bool operator!=(const _self& it) const;

		inline void ResetStartAddr(const CShmHashMap<T>* pHashMap) const
		{
			m_pStartAddr = pHashMap->m_pShmHead;
		}
		inline SShmHashNodeBase* GetValue() {
			return (SShmHashNodeBase*)((char*)m_pStartAddr+m_dwValueOffset);
		}


	private:
		mutable unsigned long m_dwValueOffset;
		mutable void* m_pStartAddr;
		//		mutable SShmHashNodeBase* m_pValue;
	};


	template<typename T>
	class CShmHashMap
	{

	public:
		typedef std::pair<const unsigned long, T> value_type;
		typedef const std::pair<const unsigned long, T> const_value_type;
		typedef value_type* pointer;
		typedef const value_type const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef unsigned long size_type;
		typedef ptrdiff_t defference_type;
		typedef SShmHashNode<T> node_type;
		typedef CShmHashMapIterator<T> iterator;
		typedef const CShmHashMapIterator<T> const_iterator;
	public:
		class CVarBuffer{

		};
	private:
#pragma pack(1)
		struct SShmHead{
			SShmHead(){
				dwShmSize = 0;
				dwMaxSize = 0;
				dwSize = 0;
				dwHashKey = 53ul;
				dwHashTableOffset = 0;
				dwEmptyHeadOffset = 0;
				dwDataHeadOffset = 0;
			}
			char szSign[128];
			size_type dwShmSize;			//共享内存大小
			size_type dwSize;				//当前使用数
			size_type dwMaxSize;			//最大可容纳数
			size_type dwHashKey;
			//node_type* pData;				//
			//node_type* pEmpty;				//empty list head
			//SShmHashNodeBase* pHashTable;			//
			unsigned long dwHashTableOffset;		//hash table head;
			unsigned long dwEmptyHeadOffset;		//empty list head
			unsigned long dwDataHeadOffset;			//data buffer 偏移量
			SShmHashNodeBase stListHead;	//list head
			size_type dwShmKey;				//shm key
			char szReserve[1020];
		};
#pragma pack()
	public:
		friend class CShmHashMapIterator<T>;


		inline CShmHashMap(void);
		inline ~CShmHashMap(void);

		inline bool init(const int iShmKey, const size_type dwShmMaxSize /* = 512*1024*/, const bool bCreate=true);
		//		inline bool fast_init(const int iShmKey, const size_type dwShmMaxSize /* = 512*1024*/, const bool bCreate=true);
		inline bool fast_init();
		inline bool uninit();
		inline bool empty() const;
		inline bool full() const;
		inline iterator begin()	
		{
			return iterator(m_pShmHead->stListHead.dwListNextOffset, m_pShmHead);
		}
		inline const_iterator begin() const {
			return iterator(m_pShmHead->stListHead.dwListNextOffset, m_pShmHead);
			//			return reinterpret_cast<SShmHashNode<T>*>(m_pListHead->pListNext);	
		}
		inline iterator end()	{	
			return iterator(this->GetOffset(&m_pShmHead->stListHead), m_pShmHead);
			//return reinterpret_cast<SShmHashNode<T>*>(m_pListHead);
		}
		inline const_iterator end() const {
			return iterator(this->GetOffset(&m_pShmHead->stListHead), m_pShmHead);
		}
		inline std::pair<iterator, bool> insert(const unsigned long dwKey,const T& tVal);
		inline std::pair<iterator, bool> insert(const value_type& value);
		inline iterator find(const unsigned long dwKey);
		//	inline const T* Find(const unsigned long dwKey);
		inline void erase(const unsigned long dwKey);
		inline void erase(iterator it);
		inline void clear();

		//test
		inline void TestEmptySize();
		inline void TestSize();

		size_type size() const {	return m_pShmHead->dwSize;	}
		size_type MaxSize() const {	return m_pShmHead->dwMaxSize;	}
		const char* GetErrMsg() const {	return m_szErrMsg;	}
		size_type GetHashKey() const {	return m_pShmHead->dwHashKey;	}
		void* GetStartAddr() const {	return m_pShmHead;	}
	private:
		inline void Free(node_type* pNode);
		inline node_type* Malloc();
		inline size_type GetHashPos(const unsigned long dwKey)
		{
			assert(m_pShmHead != NULL);
			return dwKey%m_pShmHead->dwHashKey;
		}
		inline void InsertList(node_type* pNode);
		inline void RemoveList(node_type* pNode);

		inline size_type CalcHashKey(const size_type dwMaxSize)	//计算hash key
		{
			size_type dwHashKey = dwMaxSize;
			for ( int i=__num_primes; i>0; --i )
			{
				if ( __prime_list[i-1] < dwMaxSize )
				{
					if ( i > 0 ){
						dwHashKey = __prime_list[i-1];
					}
					break;
				}
			}
			return dwHashKey;
		}
		inline char* GetAddr(const unsigned long dwOffset){
			if ( dwOffset == 0 )
				return NULL;

			assert(m_pShmHead != NULL);
			assert(dwOffset <= m_pShmHead->dwShmSize);
			return (char*)m_pShmHead + dwOffset;
		}
		inline unsigned long GetOffset(const void* ptr){
			if ( ptr == NULL )
				return 0;
			assert(m_pShmHead != NULL);
			return static_cast<unsigned long>((char*)ptr - reinterpret_cast<char*>(m_pShmHead));
		}
		inline unsigned long GetOffset(const void* ptr) const {
			if ( ptr == NULL )
				return 0;
			assert(m_pShmHead != NULL);
			return static_cast<unsigned long>((char*)ptr - reinterpret_cast<char*>(m_pShmHead));
		}
	private:
		char m_szErrMsg[1024];

		SShmHead* m_pShmHead;

		tce::CShm m_oShm;
		bool m_bInit;
	};



	template<typename T>
	CShmHashMap<T>::CShmHashMap(void)
	{
		memset(m_szErrMsg, 0, sizeof(m_szErrMsg));
		m_pShmHead = NULL;
		m_bInit = false;
	}

	template<typename T>
	CShmHashMap<T>::~CShmHashMap(void)
	{
		//if (NULL != m_pData)
		//{
		//	delete[] m_pData;
		//}

		//if (NULL != m_pHashTable)
		//{
		//	delete[] m_pHashTable;
		//}

		//if (NULL != m_pListHead)
		//{
		//	delete m_pListHead;
		//}
		this->uninit();
	}

	template<typename T>
	bool CShmHashMap<T>::fast_init()
	{
		bool bOk = false;
		if ( m_oShm.Attach() )
		{
			m_pShmHead = (SShmHead*)m_oShm.GetShmBuf();
			m_bInit = true;
			bOk = true;
		}

		return bOk;
	}


	template<typename T>
	bool CShmHashMap<T>::init(const int iShmKey, const size_type dwShmMaxSize, const bool bCreate)
	{
		if ( m_bInit )
			return true;

		//create shm
		if ( m_oShm.GetShmID() <= 0 || !m_oShm.Attach() )
		{
			if ( !m_oShm.Create(iShmKey, dwShmMaxSize, bCreate) )
			{
				tce::xsnprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm init create erro: %s", m_oShm.GetErrMsg());
				return false;
			}
		}

		if ( m_oShm.IsCreate() )
		{
			char* pShmBuf = reinterpret_cast<char*>(m_oShm.GetShmBuf());

			//shm head 
			m_pShmHead = (SShmHead*)pShmBuf;

			if ( dwShmMaxSize <= sizeof(SShmHead) )
			{
				tce::xsnprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm max size<%lu> is small.", dwShmMaxSize);
				return false;
			}

			m_pShmHead->dwShmSize = dwShmMaxSize;

			tce::xsnprintf(m_pShmHead->szSign, sizeof(m_pShmHead->szSign), "%s", SHM_HASH_SIGN_START);

			//计算可保存的记录条数
			size_type dwLeaveSize = dwShmMaxSize-sizeof(SShmHead);
			m_pShmHead->dwMaxSize = dwLeaveSize/(sizeof(node_type)+sizeof(SShmHashNodeBase));
			if ( m_pShmHead->dwMaxSize <= 0 )
			{
				tce::xsnprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm max size<%lu> is small, only store %lu record.", dwShmMaxSize, m_pShmHead->dwMaxSize);
				return false;
			}
			//计算hash key
			m_pShmHead->dwHashKey = this->CalcHashKey(m_pShmHead->dwMaxSize);

			//hash table list
			m_pShmHead->dwHashTableOffset = sizeof(SShmHead);
			SShmHashNodeBase* pHashTable = (SShmHashNodeBase*)( pShmBuf + m_pShmHead->dwHashTableOffset );
			//把hash table串起来
			for (size_type i=0; i<m_pShmHead->dwHashKey; ++i)
			{
				(pHashTable+i)->dwHashNextOffset = static_cast<unsigned long>( reinterpret_cast<char*>(pHashTable+i)  -  pShmBuf );
				(pHashTable+i)->dwHashPreOffset = static_cast<unsigned long>( reinterpret_cast<char*>(pHashTable+i)  -  pShmBuf );
			}

			m_pShmHead->stListHead.dwListNextOffset = static_cast<unsigned long>( reinterpret_cast<char*>(&m_pShmHead->stListHead) - pShmBuf );
			m_pShmHead->stListHead.dwListPreOffset = static_cast<unsigned long>( reinterpret_cast<char*>(&m_pShmHead->stListHead) - pShmBuf );


			//数据buffer 初始化
			m_pShmHead->dwDataHeadOffset = sizeof(SShmHead) + sizeof(SShmHashNodeBase)*m_pShmHead->dwHashKey;
			node_type* pDataHead = (node_type*)(m_pShmHead->dwDataHeadOffset+pShmBuf);
			memset(pDataHead, 0, sizeof(node_type)*m_pShmHead->dwMaxSize);
			//把内存连到空闲列表里
			for (size_type i=0; i<m_pShmHead->dwMaxSize; ++i)
			{
				(pDataHead+i)->dwText = i;
				if (0 == m_pShmHead->dwEmptyHeadOffset) 
				{
					(pDataHead+i)->dwHashNextOffset = 0;
					(pDataHead+i)->dwHashPreOffset = 0;
					(pDataHead+i)->dwListNextOffset = 0;
					(pDataHead+i)->dwListPreOffset = 0;
					m_pShmHead->dwEmptyHeadOffset = static_cast<unsigned long>( reinterpret_cast<char*>(pDataHead+i) - pShmBuf);
				}
				else
				{
					node_type* pEmptyHead = (node_type*)this->GetAddr(m_pShmHead->dwEmptyHeadOffset);

					(pDataHead+i)->dwListNextOffset = m_pShmHead->dwEmptyHeadOffset;//reinterpret_cast<SShmHashNodeBase*>(pEmptyHead);
					pEmptyHead->dwListPreOffset = static_cast<unsigned long>(reinterpret_cast<char*>(pDataHead+i)-pShmBuf);
					m_pShmHead->dwEmptyHeadOffset = this->GetOffset(pDataHead+i);
					(pDataHead+i)->dwListPreOffset = 0;
					(pDataHead+i)->dwHashNextOffset = 0;
					(pDataHead+i)->dwHashPreOffset = 0;
				}
			}

		}
		else
		{
			//if ( strncpy(m_pstHead->szSign, BIT_HASH_SIGN, strlen(BIT_HASH_SIGN) != 0 )
			//{
			//	tce::xsnprintf(m_szErrMsg, sizeof(m_szErrMsg), "this shm<key=%d> is no big hash map.", iShmKey);
			//	return false;
			//}

			//if ( m_pstHead->dwItemCount != dwItemCount )
			//{
			//	tce::xsnprintf(m_szErrMsg, sizeof(m_szErrMsg), "item<shm_item_count=%lu, init_item_count=%lu> count is no equal.", m_pstHead->dwItemCount, dwItemCount);
			//	return false;
			//}

			//if ( m_pstHead->dwItemSize != dwItemSize )
			//{
			//	tce::xsnprintf(m_szErrMsg, sizeof(m_szErrMsg), "item<shm_item_size=%lu, init_item_size=%lu> size is no equal.", m_pstHead->dwItemSize, dwItemSize);
			//	return false;
			//}

			//if ( m_pstHead->dwShmKey != static_cast<unsigned long>(iShmKey) )
			//{
			//	tce::xsnprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm key<shm_key=%lu, init_key=%d> is no equal.", m_pstHead->dwShmKey, iShmKey);
			//	return false;
			//}

			m_pShmHead = (SShmHead*)m_oShm.GetShmBuf();
		}

		m_bInit = true;

		return true;
	}

	template<typename T>
	bool CShmHashMap<T>::uninit()
	{
		if ( !m_bInit )
			return true;

		m_oShm.Detach();

		m_bInit = false;
		m_pShmHead = NULL;

		return true;
	}

	template<typename T>
	void CShmHashMap<T>::InsertList(node_type* pNode)
	{
		pNode->dwListNextOffset = m_pShmHead->stListHead.dwListNextOffset;// m_pListHead->pListNext;
		((node_type*)GetAddr(m_pShmHead->stListHead.dwListNextOffset))->dwListPreOffset = this->GetOffset(pNode);
		pNode->dwListPreOffset = this->GetOffset(&m_pShmHead->stListHead);
		m_pShmHead->stListHead.dwListNextOffset = this->GetOffset(pNode);
	}

	template<typename T>
	void CShmHashMap<T>::RemoveList(node_type* pNode)
	{
		assert( pNode->dwListNextOffset != this->GetOffset(pNode) );
		assert( pNode->dwListPreOffset != this->GetOffset(pNode) );
		((SShmHashNodeBase*)this->GetAddr(pNode->dwListNextOffset))->dwListPreOffset = pNode->dwListPreOffset;
		((SShmHashNodeBase*)this->GetAddr(pNode->dwListPreOffset))->dwListNextOffset = pNode->dwListNextOffset;
	}

	template<typename T>
	SShmHashNode<T>* CShmHashMap<T>::Malloc()
	{
		node_type* pNode = NULL;
		if ( this->empty() ) 
		{

			pNode = (node_type*)this->GetAddr(m_pShmHead->dwEmptyHeadOffset);
			m_pShmHead->dwEmptyHeadOffset = pNode->dwListNextOffset;
			if (0 != m_pShmHead->dwEmptyHeadOffset) 
			{
				((node_type*)this->GetAddr(m_pShmHead->dwEmptyHeadOffset))->dwListPreOffset = 0;
			}
			pNode->dwListNextOffset = 0;
			pNode->dwListPreOffset = 0;
			++m_pShmHead->dwSize;

			//加入使用列表
			this->InsertList(pNode);
		}
		else
		{
			//		xsnprintf(m_szErrMsg,sizeof(m_szErrMsg),"no space.");
		}
		return pNode;
	}

	template<typename T>
	void CShmHashMap<T>::Free(node_type* pNode)
	{
		assert(m_pShmHead!=NULL);
		this->RemoveList(pNode);
		memset(pNode,0,sizeof(node_type));
		if ( !this->empty() )
		{
			m_pShmHead->dwEmptyHeadOffset = this->GetOffset(pNode);
			pNode->dwListNextOffset = 0;
			pNode->dwListPreOffset = 0;
		}
		else
		{
			SShmHashNodeBase* pEmptyHead = (SShmHashNodeBase*)this->GetAddr(m_pShmHead->dwEmptyHeadOffset);
			pNode->dwListNextOffset = m_pShmHead->dwEmptyHeadOffset;
			pEmptyHead->dwListPreOffset = this->GetOffset(pNode);
			m_pShmHead->dwEmptyHeadOffset = pEmptyHead->dwListPreOffset;
		}
		--m_pShmHead->dwSize;

	}

	template<typename T>
	std::pair<typename CShmHashMap<T>::iterator, bool> CShmHashMap<T>::insert(const value_type& value){
		return this->insert(value.first, value.second);
	}

	template<typename T>
	std::pair<typename CShmHashMap<T>::iterator, bool>  CShmHashMap<T>::insert(const unsigned long dwKey,const T& tVal)
	{
		SShmHashNodeBase* pHashTable = (SShmHashNodeBase*)this->GetAddr(m_pShmHead->dwHashTableOffset);
		pair<CShmHashMapIterator<T>, bool> rePair(end(),false);
		size_type dwHashPos = this->GetHashPos(dwKey);


		bool bHas = false;
		SShmHashNodeBase* pHeadNode = pHashTable+dwHashPos;
		SShmHashNodeBase* pTmpNode = (SShmHashNodeBase*)GetAddr(pHeadNode->dwHashNextOffset);
		while (pHeadNode != pTmpNode)
		{
			if(reinterpret_cast<node_type*>(pTmpNode)->value.first == dwKey)
			{
				reinterpret_cast<node_type*>(pTmpNode)->value.second = tVal;
				rePair.second = true;
				rePair.first.Assign(this->GetOffset(pTmpNode), m_pShmHead);
				bHas = true;
				break;
			}
			pTmpNode = (SShmHashNodeBase*)GetAddr(pTmpNode->dwHashNextOffset);
		}

		if (!bHas)
		{
			SShmHashNodeBase* pNewNode = reinterpret_cast<SShmHashNodeBase*>(this->Malloc());
			if (NULL != pNewNode)
			{
				*(unsigned long*)&(reinterpret_cast<node_type*>(pNewNode)->value.first) = dwKey;
				reinterpret_cast<node_type*>(pNewNode)->value.second = tVal;		
				rePair.second = true;
				rePair.first.Assign(this->GetOffset(pTmpNode), m_pShmHead);

				pNewNode->dwHashNextOffset = pHeadNode->dwHashNextOffset;
				((node_type*)GetAddr(pHeadNode->dwHashNextOffset))->dwHashPreOffset = this->GetOffset(pNewNode);
				pNewNode->dwHashPreOffset = this->GetOffset(pHeadNode);
				pHeadNode->dwHashNextOffset = this->GetOffset(pNewNode);
			}
			else
			{
				rePair.second = false;
			}
		}

		return rePair;
	}

	template<typename T>
	CShmHashMapIterator<T> CShmHashMap<T>::find(const unsigned long dwKey)
	{
		SShmHashNodeBase* pHashTable = (SShmHashNodeBase*)this->GetAddr(m_pShmHead->dwHashTableOffset);
		SShmHashNodeBase* pVal = &m_pShmHead->stListHead;
		size_type dwHashPos = this->GetHashPos(dwKey);
		SShmHashNodeBase* pHeadNode = pHashTable+dwHashPos;
		node_type* pTmpNode = (node_type*)GetAddr(pHeadNode->dwHashNextOffset);
		while (pHeadNode != pTmpNode)
		{
			if(pTmpNode->value.first == dwKey)
			{
				pVal = pTmpNode;
				break;
			}
			pTmpNode = (node_type*)GetAddr(pTmpNode->dwHashNextOffset);
		}
		return	iterator(this->GetOffset(pVal), m_pShmHead);

		//		SShmHashNodeBase* pHashTable = (SShmHashNodeBase*)this->GetAddr(m_pShmHead->dwHashTableOffset);
		//		//SShmHashNodeBase* pVal = &m_pShmHead->stListHead;
		//		size_type dwHashPos = this->GetHashPos(dwKey);
		//		SShmHashNodeBase* pHeadNode = pHashTable+dwHashPos;
		//		unsigned long dwTmpNodeOffset = pHeadNode->dwHashNextOffset;
		//		unsigned long dwHeadNodeOffset = this->GetOffset(pHeadNode);
		//		unsigned long dwValOffset = GetOffset(&m_pShmHead->stListHead);
		////		SShmHashNodeBase* pTmpNode = (SShmHashNodeBase*)GetAddr(pHeadNode->dwHashNextOffset);
		//		while (dwHeadNodeOffset != dwTmpNodeOffset)
		//		{
		//			if(reinterpret_cast<node_type*>(GetAddr(dwTmpNodeOffset))->value.first == dwKey)
		//			{
		//				dwValOffset = dwTmpNodeOffset;
		//				break;
		//			}
		//			dwTmpNodeOffset = ((SShmHashNodeBase*)GetAddr(dwTmpNodeOffset))->dwHashNextOffset;
		////			pTmpNode = (SShmHashNodeBase*)GetAddr(pTmpNode->dwHashNextOffset);
		//		}
		//		return	iterator(dwValOffset, m_pShmHead);

	}

	template<typename T>
	void CShmHashMap<T>::erase(const unsigned long dwKey)
	{
		SShmHashNodeBase* pHashTable = (SShmHashNodeBase*)this->GetAddr(m_pShmHead->dwHashTableOffset);
		size_type dwHashPos = this->GetHashPos(dwKey);
		SShmHashNodeBase* pHeadNode = pHashTable+dwHashPos;
		SShmHashNodeBase* pTmpNode = (SShmHashNodeBase*)this->GetAddr(pHeadNode->dwHashNextOffset);
		while (pHeadNode != pTmpNode)
		{
			if(reinterpret_cast<node_type*>(pTmpNode)->value.first == dwKey)
			{
				((node_type*)GetAddr(pTmpNode->dwHashNextOffset))->dwHashPreOffset = pTmpNode->dwHashPreOffset;
				((node_type*)GetAddr(pTmpNode->dwHashPreOffset))->dwHashNextOffset = pTmpNode->dwHashNextOffset;
				this->Free(reinterpret_cast<node_type*>(pTmpNode));
				break;
			}
			pTmpNode = (node_type*)GetAddr(pTmpNode->dwHashNextOffset);
		}
	}

	template<typename T>
	void CShmHashMap<T>::erase(iterator it)
	{
		if (it != this->end())
		{
			SShmHashNodeBase* pTmpNode = it.GetValue();
			((node_type*)GetAddr(pTmpNode->dwHashNextOffset))->dwHashPreOffset = pTmpNode->dwHashPreOffset;
			((node_type*)GetAddr(pTmpNode->dwHashPreOffset))->dwHashNextOffset = pTmpNode->dwHashNextOffset;
			this->Free(reinterpret_cast<node_type*>(pTmpNode));
		}
	}

	template<typename T>
	void CShmHashMap<T>::clear()
	{
		assert(m_pShmHead != NULL);
		SShmHashNodeBase* pHashTable = (SShmHashNodeBase*)this->GetAddr(m_pShmHead->dwHashTableOffset);
		for (size_type i=0; i<m_pShmHead->dwHashKey; ++i)
		{
			(pHashTable+i)->dwHashNextOffset = GetOffset(pHashTable+i);
			(pHashTable+i)->dwHashPreOffset = GetOffset(pHashTable+i);
		}

		m_pShmHead->dwSize = 0;
		SShmHashNodeBase* pListHead = &m_pShmHead->stListHead;
		if ( (node_type*)GetAddr(pListHead->dwListNextOffset) != pListHead)
		{

			if ( NULL != GetAddr(m_pShmHead->dwEmptyHeadOffset) )
			{
				((node_type*)GetAddr(pListHead->dwListNextOffset))->dwListPreOffset = 0;
				((node_type*)GetAddr(pListHead->dwListPreOffset))->dwListNextOffset = m_pShmHead->dwEmptyHeadOffset;
				m_pShmHead->dwEmptyHeadOffset = pListHead->dwListNextOffset;
			}
			else
			{
				m_pShmHead->dwEmptyHeadOffset = pListHead->dwListNextOffset;
				((node_type*)GetAddr(pListHead->dwListNextOffset))->dwListPreOffset = 0;
				((node_type*)GetAddr(pListHead->dwListPreOffset))->dwListNextOffset = 0;
			}

			pListHead->dwListNextOffset = pListHead->dwListPreOffset = this->GetOffset(pListHead);
		}
	}

	template<typename T>
	bool CShmHashMap<T>::empty() const
	{
		bool bEmpty = m_pShmHead->dwEmptyHeadOffset == 0 ? false : true;
#ifndef NDEBUG
		if ( bEmpty )
			//		if (m_pListHead->pListNext == m_pListHead)
		{
			assert(m_pShmHead->dwSize != m_pShmHead->dwMaxSize);
		}
		else
		{
			assert(m_pShmHead->dwSize == m_pShmHead->dwMaxSize);
		}
#endif

		return bEmpty ? true : false;
	}

	template<typename T>
	bool CShmHashMap<T>::full() const
	{
		bool bEmpty = this->empty();
#ifndef NDEBUG
		if ( !bEmpty )
		{
			assert(m_pShmHead->dwSize == m_pShmHead->dwMaxSize);
		}
		else
		{
			assert(m_pShmHead->dwSize != m_pShmHead->dwMaxSize);
		}
#endif
		return bEmpty ? false : true;
	}

	//test
	template<typename T>
	void CShmHashMap<T>::TestEmptySize()
	{
#ifndef NDEBUG
		size_type tmpSize = 0;

		SShmHashNodeBase* pTmpEmptyHead = reinterpret_cast<SShmHashNodeBase*>(this->GetAddr(m_pShmHead->dwEmptyHeadOffset));

		while(pTmpEmptyHead)
		{
			tmpSize++;
			pTmpEmptyHead = reinterpret_cast<SShmHashNodeBase*>(this->GetAddr(pTmpEmptyHead->dwListNextOffset));
		}

		assert(tmpSize == m_pShmHead->dwMaxSize-m_pShmHead->dwSize);
#endif
	}

	template<typename T>
	void CShmHashMap<T>::TestSize()
	{
#ifndef NDEBUG
		size_type tmpSize = 0;

		SShmHashNodeBase* pTmpListHead = &m_pShmHead->stListHead;
		unsigned long dwListHeadOffset = this->GetOffset(&m_pShmHead->stListHead);
		while(pTmpListHead->dwListNextOffset != dwListHeadOffset)
		{
			tmpSize++;
			pTmpListHead = (SShmHashNodeBase*)this->GetAddr(pTmpListHead->dwListNextOffset);
		}

		assert(tmpSize == m_pShmHead->dwSize);
#endif
	}


	//////////////////////////////////////////////////////////////////////////
	//HashMap iterator

	template<typename T>
	CShmHashMapIterator<T>::CShmHashMapIterator()
		:m_dwValueOffset(0)
		,m_pStartAddr(NULL)
	{
	}

	template<typename T>
	CShmHashMapIterator<T>::~CShmHashMapIterator()
	{
	}

	template<typename T>
	CShmHashMapIterator<T>::CShmHashMapIterator(const unsigned long dwValueOffset, void* pStartAddr)
		:m_dwValueOffset(dwValueOffset)
		,m_pStartAddr(pStartAddr)
	{
	}

	template<typename T>
	CShmHashMapIterator<T>& CShmHashMapIterator<T>::operator=(const CShmHashMapIterator<T>& rhs)
	{
		if (this != &rhs)
		{
			memcpy(this, &rhs, sizeof(rhs));
			//m_dwValueOffset = rhs.m_dwValueOffset;
			//m_pStartAddr = rhs.m_pStartAddr;

		}
		return *this;
	}

	//template<typename T>
	//CShmHashMapIterator<T>& CShmHashMapIterator<T>::operator=(SShmHashNodeBase* pValue)
	//{
	//	m_pValue = pValue;
	//	return *this;
	//}

	template<typename T>
	CShmHashMapIterator<T>& CShmHashMapIterator<T>::Assign(const unsigned long dwValueOffset, void* pStartAddr)
	{
		m_dwValueOffset = dwValueOffset;
		m_pStartAddr = pStartAddr;
		return *this;
	}

	template<typename T>
	CShmHashMapIterator<T>::CShmHashMapIterator(const CShmHashMapIterator<T>& rhs)
	{
		memcpy(this, &rhs, sizeof(rhs));

		//m_dwValueOffset = rhs.m_dwValueOffset;
		//m_pStartAddr = rhs.m_pStartAddr;
	}

	template<typename T>
	const CShmHashMapIterator<T>& CShmHashMapIterator<T>::operator++() const
	{
		m_dwValueOffset = ((SShmHashNodeBase*)((char*)m_pStartAddr+m_dwValueOffset))->dwListNextOffset;
		return *this;
	}

	template<typename T>
	CShmHashMapIterator<T>& CShmHashMapIterator<T>::operator++() 
	{
		m_dwValueOffset = ((SShmHashNodeBase*)((char*)m_pStartAddr+m_dwValueOffset))->dwListNextOffset;
		return *this;
	}
	template<typename T>
	CShmHashMapIterator<T> CShmHashMapIterator<T>::operator++(int)
	{
		CShmHashMapIterator<T> tmp = *this;

		m_dwValueOffset = ((SShmHashNodeBase*)((char*)m_pStartAddr+m_dwValueOffset))->dwListNextOffset;
		return tmp;
	}
	template<typename T>
	const CShmHashMapIterator<T> CShmHashMapIterator<T>::operator++(int) const
	{
		CShmHashMapIterator<T> tmp = *this;
		m_dwValueOffset = ((SShmHashNodeBase*)((char*)m_pStartAddr+m_dwValueOffset))->dwListNextOffset;
		return tmp;
	}

	template<typename T>
	CShmHashMapIterator<T>& CShmHashMapIterator<T>::operator--()
	{
		m_dwValueOffset = ((SShmHashNodeBase*)((char*)m_pStartAddr+m_dwValueOffset))->dwListPreOffset;
		return *this;
	}

	template<typename T>
	CShmHashMapIterator<T> CShmHashMapIterator<T>::operator--(int)
	{
		CShmHashMapIterator<T> tmp = *this;
		m_dwValueOffset = ((SShmHashNodeBase*)((char*)m_pStartAddr+m_dwValueOffset))->dwListPreOffset;
		return tmp;
	}



	template<typename T>
	std::pair<const unsigned long, T>& CShmHashMapIterator<T>::operator*()
	{
		return ((SShmHashNode<T>*)((char*)m_pStartAddr+m_dwValueOffset))->value;
	}

	template<typename T>
	const std::pair<const unsigned long, T>* CShmHashMapIterator<T>::operator->() const
	{
		return &(((SShmHashNode<T>*)((char*)m_pStartAddr+m_dwValueOffset))->value);
	}
	template<typename T>
	std::pair<const unsigned long, T>* CShmHashMapIterator<T>::operator->()
	{

		return &(((SShmHashNode<T>*)((char*)m_pStartAddr+m_dwValueOffset))->value);
	}

	template<typename T>
	std::pair<const unsigned long, T> * CShmHashMapIterator<T>::operator&()
	{
		return &(((SShmHashNode<T>*)((char*)m_pStartAddr+m_dwValueOffset))->value);
	}

	template<typename T>
	bool CShmHashMapIterator<T>::operator==(const CShmHashMapIterator<T>& it) const
	{
		return m_dwValueOffset == it.m_dwValueOffset;
	}

	template<typename T>
	bool CShmHashMapIterator<T>::operator!=(const CShmHashMapIterator<T>& it) const
	{
		return m_dwValueOffset != it.m_dwValueOffset;
	}

};





#endif

