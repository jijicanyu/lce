#ifndef __NCE_SHM_LIST_H__
#define __NCE_SHM_LIST_H__

#include <utility>
#include <memory>
#include <cassert>
#include "CShm.h"
#include <iostream>

using namespace std;
namespace lce
{



template<typename T>
class CShmList;

typedef unsigned long size_type;

#pragma pack(1)
	struct SShmHead{
		SShmHead(){
			dwShmSize = 0;
			dwMaxSize = 0;
			dwSize = 0;
			dwListHeadOffset = 0;
			dwListTailOffset = 0;
			dwEmptyHeadOffset = 0;
			dwUseHeadOffset = 0;
			dwDataOffset = 0;
		}
		char szSign[128];
		size_type dwShmSize;			//共享内存大小
		size_type dwSize;				//当前使用数
		size_type dwMaxSize;			//最大可容纳数
		unsigned long dwDataOffset; //数据起始位置
		unsigned long dwListHeadOffset;		//list table head;
		unsigned long dwListTailOffset;		//list table tail;
		unsigned long dwEmptyHeadOffset;		//empty list head
		unsigned long dwUseHeadOffset;
		size_type dwShmKey;				//shm key
		char szReserve[64];
	};
#pragma pack()


	#define SHM_LIST_SIGN_START  "lce shm list head ver 1.0 start"
	#define SHM_LIST_SIGN_END  "lce shml list head ver 1.0 end"

	template<typename T>
	struct SShmListNode{
		unsigned long dwListNextOffset;	//LIST列表使用(地址偏移量)
		unsigned long dwListPreOffset;	//list列表使用(地址偏移量)
		unsigned long dwEmptyListNextOffset;	//空闲或使用列表指针使用(地址偏移量)
		T tVal;
	};


	//iterator
	template<typename T>
	class CShmListIterator
	{
	public:
		typedef CShmListIterator<T> _self;
		typedef T value_type;
	public:
		inline CShmListIterator();
		inline ~CShmListIterator();
		inline CShmListIterator(const unsigned long dwValueOffset, void* pStartAddr);
		inline _self& operator=(const _self& rhs);
		inline _self& assign(const unsigned long dwValueOffset, void* pStartAddr);
		inline CShmListIterator(const _self& rhs);
		inline const _self& operator++() const;
		inline _self& operator++() ;
		inline _self operator++(int);
		inline const _self operator++(int) const;
		inline _self& operator--();
		inline _self operator--(int);
		inline value_type& operator*();
		inline value_type* operator&();
		inline const value_type* operator->() const;
		inline value_type* operator->();
		bool operator==(const _self& it) const;
		bool operator!=(const _self& it) const;

		inline void resetStartAddr(const CShmList<T>* pList) const
		{
			m_pStartAddr = pList->m_pShmHead;
		}
		inline SShmListNode<T>* getValue() {
			return (SShmListNode<T>*)((char*)m_pStartAddr+m_dwValueOffset);
		}
	private:
		mutable unsigned long m_dwValueOffset;
		mutable void* m_pStartAddr;
	};


	//iterator
	template<typename T>
	class CShmListReverseIterator
	{
	public:
		typedef CShmListReverseIterator<T> _self;
		typedef T value_type;
	public:
		inline CShmListReverseIterator();
		inline ~CShmListReverseIterator();
		inline CShmListReverseIterator(const unsigned long dwValueOffset, void* pStartAddr);
		inline _self& operator=(const _self& rhs);
		inline _self& assign(const unsigned long dwValueOffset, void* pStartAddr);
		inline CShmListReverseIterator(const _self& rhs);
		inline const _self& operator++() const;
		inline _self& operator++() ;
		inline _self operator++(int);
		inline const _self operator++(int) const;
		inline _self& operator--();
		inline _self operator--(int);
		inline value_type& operator*();
		inline value_type* operator&();
		inline const value_type* operator->() const;
		inline value_type* operator->();
		bool operator==(const _self& it) const;
		bool operator!=(const _self& it) const;

		inline void resetStartAddr(const CShmList<T>* pList) const
		{
			m_pStartAddr = pList->m_pShmHead;
		}
		inline SShmListNode<T>* getValue() {
			return (SShmListNode<T>*)((char*)m_pStartAddr+m_dwValueOffset);
		}
	private:
		mutable unsigned long m_dwValueOffset;
		mutable void* m_pStartAddr;
	};


	template<typename T>
	class CShmList
	{

	public: 
		typedef SShmListNode<T>  node_type;
		typedef  T value_type;
		typedef const  T const_value_type;
		typedef value_type* pointer;
		typedef const value_type const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef unsigned long size_type;
		typedef ptrdiff_t defference_type;

		typedef CShmListIterator<T> iterator;
		typedef const CShmListIterator<T> const_iterator;

		typedef CShmListReverseIterator<T> reverse_iterator;
		typedef const CShmListReverseIterator<T> const_reverse_iterator;
	public:
		CShmList(void);
		~CShmList(){this->uninit();}
		bool init(const int iShmKey, const size_type dwShmMaxSize, const bool bCreate);
		bool uninit();
		size_type size() const {	return m_pShmHead->dwSize;	}
		size_type max_size() const {	return m_pShmHead->dwMaxSize;	}
		bool empty();
		bool full();

		bool push_back(const T& tVal);
		bool push_front(const T& tVal);
		T & front();
		T & back();

		const char* getErrMsg() const {	return m_szErrMsg;	}

		
		void pop_back();
		void pop_front();
		
		bool insert(iterator it,const T& tVal);

		const_iterator begin() const
		{
			if(empty())
			{
				return iterator(((node_type*)this->getAddr(m_pShmHead->dwListTailOffset))->dwListNextOffset, m_pShmHead);
			}
			else
			{
				return iterator(m_pShmHead->dwListHeadOffset, m_pShmHead);
			}

		}

		const_iterator end() const
		{
			return iterator(((node_type*)this->getAddr(m_pShmHead->dwListTailOffset))->dwListNextOffset, m_pShmHead);
		}

		iterator begin()
		{
			if(empty())
			{
				return iterator(((node_type*)this->getAddr(m_pShmHead->dwListTailOffset))->dwListNextOffset, m_pShmHead);
			}
			else
			{
				return iterator(m_pShmHead->dwListHeadOffset, m_pShmHead);
			}
		}

		iterator end()
		{
			return iterator(((node_type*)this->getAddr(m_pShmHead->dwListTailOffset))->dwListNextOffset, m_pShmHead);
		}

		reverse_iterator rbegin()
		{
			if(empty())
			{
				return reverse_iterator(((node_type*)this->getAddr(m_pShmHead->dwListHeadOffset))->dwListPreOffset, m_pShmHead);
			}
			else
			{
				return reverse_iterator(m_pShmHead->dwListTailOffset, m_pShmHead);
			}

		}
		reverse_iterator rend()
		{
			return reverse_iterator(((node_type*)this->getAddr(m_pShmHead->dwListHeadOffset))->dwListPreOffset, m_pShmHead);
		}
		void erase(iterator it);
		void clear();

	private:
		SShmListNode<T>* malloc();
		void free(node_type* pNode);

	private:
		inline char* getAddr(const unsigned long dwOffset)
		{
			if ( dwOffset == 0 )
				return NULL;

			return (char*)m_pShmHead + dwOffset;
		}
		inline unsigned long getOffset(const void* ptr)
		{
			if ( ptr == NULL )
				return 0;
			return static_cast<unsigned long>((char*)ptr - reinterpret_cast<char*>(m_pShmHead));
		}
		inline unsigned long getOffset(const void* ptr) const 
		{
			if ( ptr == NULL )
				return 0;
			return static_cast<unsigned long>((char*)ptr - reinterpret_cast<char*>(m_pShmHead));
		}
	private:
		char m_szErrMsg[1024];

		SShmHead* m_pShmHead;

		lce::CShm m_oShm;
		bool m_bInit;

	};

	template<typename T>
	CShmList<T>::CShmList(void)
	{
		memset(m_szErrMsg, 0, sizeof(m_szErrMsg));
		m_pShmHead = NULL;
		m_bInit = false;
	}

	template<typename T>
	bool CShmList<T>::init(const int iShmKey, const size_type dwShmMaxSize, const bool bCreate)
	{
		if ( m_bInit )
			return true;

		//create shm
		if ( m_oShm.getShmID() <= 0 || !m_oShm.attach() )
		{
			if ( !m_oShm.create(iShmKey, dwShmMaxSize, bCreate) )
			{
				snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm init create erro: %s", m_oShm.getErrMsg());
				return false;
			}
		}

		if ( m_oShm.isCreate() )
		{
			char* pShmBuf = reinterpret_cast<char*>(m_oShm.getShmBuf());

			//shm head 
			m_pShmHead = (SShmHead*)pShmBuf;
			memset(m_pShmHead,0,sizeof(SShmHead));

			if ( dwShmMaxSize <= sizeof(SShmHead) )
			{
				snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm max size<%lu> is small.", dwShmMaxSize);
				return false;
			}
			m_pShmHead->dwShmSize = dwShmMaxSize;

			snprintf(m_pShmHead->szSign, sizeof(m_pShmHead->szSign), "%s", SHM_LIST_SIGN_START);

			//计算可保存的记录条数
			size_type dwLeaveSize = dwShmMaxSize-sizeof(SShmHead);
			m_pShmHead->dwMaxSize = dwLeaveSize/(sizeof(node_type));

			if ( m_pShmHead->dwMaxSize <= 0 )
			{
				snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm max size<%lu> is small, only store %lu record.", dwShmMaxSize, m_pShmHead->dwMaxSize);
				return false;
			}

			m_pShmHead->dwDataOffset = sizeof(SShmHead);
			//数据buffer 初始化
			node_type* pStart = (node_type*)(pShmBuf+m_pShmHead->dwDataOffset);
			memset(pStart, 0, sizeof(node_type)*m_pShmHead->dwMaxSize);

			//把内存连到空闲列表里
			for (size_type i=0; i<m_pShmHead->dwMaxSize; ++i)
			{
				node_type* pNode =pStart+i;

				if (0 == m_pShmHead->dwEmptyHeadOffset) 
				{
					pNode->dwListNextOffset = 0;
					pNode->dwListPreOffset = 0;
					pNode->dwEmptyListNextOffset = 0;
					m_pShmHead->dwEmptyHeadOffset = static_cast<unsigned long>( reinterpret_cast<char*>(pNode) - pShmBuf);
	
				}
				else
				{
					node_type* pEmptyHead = (node_type*)this->getAddr(m_pShmHead->dwEmptyHeadOffset);

					pNode->dwEmptyListNextOffset = m_pShmHead->dwEmptyHeadOffset;
					m_pShmHead->dwEmptyHeadOffset = this->getOffset(pNode);
	
					pNode->dwListPreOffset = 0;
					pNode->dwListNextOffset = 0;

				}
			}

		}
		else
		{
			m_pShmHead = (SShmHead*)m_oShm.getShmBuf();
		}

		m_bInit = true;

		return true;
	}

	template<typename T>
	bool CShmList<T>::uninit()
	{
		if ( !m_bInit )
			return true;

		m_oShm.detach();

		m_bInit = false;
		m_pShmHead = NULL;

		return true;
	}

	template<typename T>
	bool CShmList<T>::push_back(const T& tVal)
	{
		if(empty())
		{
				node_type *pNewNode = this->malloc();
				if(pNewNode == NULL) return false;

				pNewNode->dwListPreOffset = 0;
				pNewNode->dwListNextOffset = 0;
				m_pShmHead->dwListTailOffset = this->getOffset(pNewNode);
				m_pShmHead->dwListHeadOffset = this->getOffset(pNewNode);
				pNewNode->tVal = tVal;

		}
		else
		{
			node_type *pListTail= (node_type *)this->getAddr(m_pShmHead->dwListTailOffset);
			node_type *pNewNode = this->malloc();
			if(pNewNode == NULL) return false;

			pListTail->dwListNextOffset = this->getOffset(pNewNode);
			pNewNode->dwListPreOffset = m_pShmHead->dwListTailOffset;
			pNewNode->dwListNextOffset = 0;
			m_pShmHead->dwListTailOffset = pListTail->dwListNextOffset;

			pNewNode->tVal = tVal;
		}
		return true;
	}
	
	template<typename T>
	bool CShmList<T>::push_front(const T& tVal)
	{

		if(empty())
		{
			node_type *pNewNode = this->malloc();

			if(pNewNode == NULL) return false;

			pNewNode->dwListPreOffset = 0;
			pNewNode->dwListNextOffset = 0;
			m_pShmHead->dwListTailOffset = this->getOffset(pNewNode);
			m_pShmHead->dwListHeadOffset = this->getOffset(pNewNode);
			pNewNode->tVal = tVal;
		}
		else
		{
			node_type *pListHead= (node_type *)this->getAddr(m_pShmHead->dwListHeadOffset);
			node_type *pNewNode = this->malloc();

			if(pNewNode == NULL) return false;

			pListHead->dwListPreOffset = this->getOffset(pNewNode);
			pNewNode->dwListNextOffset = m_pShmHead->dwListHeadOffset;
			pNewNode->dwListPreOffset = 0;
			m_pShmHead->dwListHeadOffset = pListHead->dwListPreOffset;
			pNewNode->tVal = tVal;
		}
		return true;
	}

	template<typename T>
	bool CShmList<T>::insert(iterator it,const T& tVal)
	{

		if(empty())	return false;

		node_type *pNode = it.getValue();
		node_type *pNewNode = this->malloc();

		if(pNewNode == NULL) return false;

		pNewNode->dwListNextOffset = pNode ->dwListNextOffset;
		pNewNode->dwListPreOffset = getOffset(pNode);
		pNode->dwListNextOffset = getOffset(pNewNode);
		((node_type *)getAddr(pNewNode->dwListNextOffset))->dwListPreOffset = pNode->dwListNextOffset;

		if(m_pShmHead->dwListTailOffset == getOffset(pNode))
		{
			m_pShmHead->dwListTailOffset = pNode->dwListNextOffset; 
		}

		pNewNode->tVal = tVal;

		return true;
	}


	template<typename T>
	T & CShmList<T>::front()
	{
		return ((node_type *)this->getAddr(m_pShmHead->dwListHeadOffset))->tVal;
	}

	template<typename T>
	T & CShmList<T>::back()
	{
		return ((node_type *)this->getAddr(m_pShmHead->dwListTailOffset))->tVal;
	}

	template<typename T>
	void CShmList<T>::erase(iterator it)
	{
		if(empty())	return ;

		node_type *pNode = it.getValue();

		if(getOffset(pNode) == m_pShmHead->dwListHeadOffset)
		{
			m_pShmHead->dwListHeadOffset = pNode->dwListNextOffset;
			((node_type*)getAddr(pNode->dwListNextOffset))->dwListPreOffset = pNode->dwListPreOffset;
		}
		else if(getOffset(pNode) == m_pShmHead->dwListTailOffset)
		{
			m_pShmHead->dwListTailOffset = pNode->dwListPreOffset;
			((node_type*)getAddr(pNode->dwListPreOffset))->dwListNextOffset = pNode->dwListNextOffset;
		}
		else
		{
			((node_type*)getAddr(pNode->dwListPreOffset))->dwListNextOffset = pNode->dwListNextOffset;
			((node_type*)getAddr(pNode->dwListNextOffset))->dwListPreOffset = pNode->dwListPreOffset;
		}

		free(pNode);

	}

	template<typename T>
	void CShmList<T>::clear()
	{

		m_pShmHead->dwDataOffset = sizeof(SShmHead);
		//数据buffer 初始化
		node_type* pStart = (node_type *)getAddr(m_pShmHead->dwDataOffset);
		memset(pStart, 0, sizeof(node_type)*m_pShmHead->dwMaxSize);
		m_pShmHead->dwEmptyHeadOffset = 0;
		m_pShmHead->dwSize = 0;

		//把内存连到空闲列表里
		for (size_type i=0; i<m_pShmHead->dwMaxSize; ++i)
		{
			node_type* pNode =pStart+i;

			if (0 == m_pShmHead->dwEmptyHeadOffset) 
			{
				pNode->dwListNextOffset = 0;
				pNode->dwListPreOffset = 0;
				pNode->dwEmptyListNextOffset = 0;
				m_pShmHead->dwEmptyHeadOffset = getOffset(pNode);

			}
			else
			{
				node_type* pEmptyHead = (node_type*)this->getAddr(m_pShmHead->dwEmptyHeadOffset);

				pNode->dwEmptyListNextOffset = m_pShmHead->dwEmptyHeadOffset;
				m_pShmHead->dwEmptyHeadOffset = this->getOffset(pNode);
				pNode->dwListPreOffset = 0;
				pNode->dwListNextOffset = 0;
			}
		}
	}


	template<typename T>
	void CShmList<T>::pop_front()
	{

		if(empty())	return ;

		erase(begin());
		
	}

	template<typename T>
	void CShmList<T>::pop_back()
	{

		if(empty())	return ;

		erase(iterator(m_pShmHead->dwListTailOffset, m_pShmHead));
	}

	template<typename T>
	SShmListNode<T>* CShmList<T>::malloc()
	{
		node_type* pNode = NULL;

		if ( !this->full() ) 
		{

			pNode = (node_type*)this->getAddr(m_pShmHead->dwEmptyHeadOffset);
			m_pShmHead->dwEmptyHeadOffset = pNode->dwEmptyListNextOffset;
			/*
			if (0 != m_pShmHead->dwEmptyHeadOffset) 
			{
				((node_type*)this->getAddr(m_pShmHead->dwEmptyHeadOffset))->dwEmptyListPreOffset = 0;
			}
			*/
			pNode->dwEmptyListNextOffset = 0;
			++m_pShmHead->dwSize;

		}
		else
		{
			snprintf(m_szErrMsg,sizeof(m_szErrMsg),"no space.");
		}

		//cout<<"moffset="<<(unsigned long)this->getOffset(pNode)<<endl;
		return pNode;
	}

	template<typename T>
	void CShmList<T>::free(node_type* pNode)
	{

		//cout<<"foffset="<<(unsigned long)this->getOffset(pNode)<<endl;
		memset(pNode,0,sizeof(node_type));
        
		if(m_pShmHead->dwEmptyHeadOffset == 0)
		{
			m_pShmHead->dwEmptyHeadOffset = this->getOffset(pNode);

		}
		else
		{
			node_type* pEmptyHead = (node_type*)this->getAddr(m_pShmHead->dwEmptyHeadOffset);
			pNode->dwEmptyListNextOffset = m_pShmHead->dwEmptyHeadOffset;
			//pEmptyHead->dwEmptyListPreOffset = this->getOffset(pNode);
			m_pShmHead->dwEmptyHeadOffset = this->getOffset(pNode);
		}
	

		--m_pShmHead->dwSize;

	}


	template<typename T>
	bool CShmList<T>::empty() 
	{
		return m_pShmHead->dwSize == 0;
	}

	template<typename T>
	bool CShmList<T>::full() 
	{
		return m_pShmHead->dwSize >= m_pShmHead->dwMaxSize;
	}




	template<typename T>
	CShmListIterator<T>::CShmListIterator()
		:m_dwValueOffset(0)
		,m_pStartAddr(NULL)
	{
	}

	template<typename T>
	CShmListIterator<T>::~CShmListIterator()
	{
	}

	template<typename T>
	CShmListIterator<T>::CShmListIterator(const unsigned long dwValueOffset, void* pStartAddr)
		:m_dwValueOffset(dwValueOffset)
		,m_pStartAddr(pStartAddr)
	{
	}

	template<typename T>
	CShmListIterator<T>& CShmListIterator<T>::operator=(const CShmListIterator<T>& rhs)
	{
		if (this != &rhs)
		{
			memcpy(this, &rhs, sizeof(rhs));
		}
		return *this;
	}

	template<typename T>
	CShmListIterator<T>& CShmListIterator<T>::assign(const unsigned long dwValueOffset, void* pStartAddr)
	{
		m_dwValueOffset = dwValueOffset;
		m_pStartAddr = pStartAddr;
		return *this;
	}

	template<typename T>
	CShmListIterator<T>::CShmListIterator(const CShmListIterator<T>& rhs)
	{
		memcpy(this, &rhs, sizeof(rhs));
	}

	template<typename T>
	const CShmListIterator<T>& CShmListIterator<T>::operator++() const
	{
		m_dwValueOffset = ((SShmListNode<T>*)((char*)m_pStartAddr+m_dwValueOffset))->dwListNextOffset;
		return *this;
	}

	template<typename T>
	CShmListIterator<T>& CShmListIterator<T>::operator++() 
	{
		m_dwValueOffset = ((SShmListNode<T>*)((char*)m_pStartAddr+m_dwValueOffset))->dwListNextOffset;
		return *this;
	}
	template<typename T>
	CShmListIterator<T> CShmListIterator<T>::operator++(int)
	{
		CShmListIterator<T> tmp = *this;

		m_dwValueOffset = ((SShmListNode<T>*)((char*)m_pStartAddr+m_dwValueOffset))->dwListNextOffset;
		return tmp;
	}
	template<typename T>
	const CShmListIterator<T> CShmListIterator<T>::operator++(int) const
	{
		CShmListIterator<T> tmp = *this;
		m_dwValueOffset = ((SShmListNode<T>*)((char*)m_pStartAddr+m_dwValueOffset))->dwListNextOffset;
		return tmp;
	}

	template<typename T>
	CShmListIterator<T>& CShmListIterator<T>::operator--()
	{
		m_dwValueOffset = ((SShmListNode<T>*)((char*)m_pStartAddr+m_dwValueOffset))->dwListPreOffset;
		return *this;
	}

	template<typename T>
	CShmListIterator<T> CShmListIterator<T>::operator--(int)
	{
		CShmListIterator<T> tmp = *this;
		m_dwValueOffset = ((SShmListNode<T>*)((char*)m_pStartAddr+m_dwValueOffset))->dwListPreOffset;
		return tmp;
	}



	template<typename T>
	T & CShmListIterator<T>::operator*()
	{
		return ((SShmListNode<T>*)((char*)m_pStartAddr+m_dwValueOffset))->tVal;
	}

	template<typename T>
	const T* CShmListIterator<T>::operator->() const
	{
		return &(((SShmListNode<T>*)((char*)m_pStartAddr+m_dwValueOffset))->tVal);
	}
	template<typename T>
	T* CShmListIterator<T>::operator->()
	{

		return &(((SShmListNode<T>*)((char*)m_pStartAddr+m_dwValueOffset))->tVal);
	}

	template<typename T>
	T * CShmListIterator<T>::operator&()
	{
		return &(((SShmListNode<T>*)((char*)m_pStartAddr+m_dwValueOffset))->tVal);
	}

	template<typename T>
	bool CShmListIterator<T>::operator==(const CShmListIterator<T>& it) const
	{
		return m_dwValueOffset == it.m_dwValueOffset;
	}

	template<typename T>
	bool CShmListIterator<T>::operator!=(const CShmListIterator<T>& it) const
	{
		return m_dwValueOffset != it.m_dwValueOffset;
	}




	template<typename T>
	CShmListReverseIterator<T>::CShmListReverseIterator()
		:m_dwValueOffset(0)
		,m_pStartAddr(NULL)
	{
	}

	template<typename T>
	CShmListReverseIterator<T>::~CShmListReverseIterator()
	{
	}

	template<typename T>
	CShmListReverseIterator<T>::CShmListReverseIterator(const unsigned long dwValueOffset, void* pStartAddr)
		:m_dwValueOffset(dwValueOffset)
		,m_pStartAddr(pStartAddr)
	{
	}

	template<typename T>
	CShmListReverseIterator<T>& CShmListReverseIterator<T>::operator=(const CShmListReverseIterator<T>& rhs)
	{
		if (this != &rhs)
		{
			memcpy(this, &rhs, sizeof(rhs));
		}
		return *this;
	}

	template<typename T>
	CShmListReverseIterator<T>& CShmListReverseIterator<T>::assign(const unsigned long dwValueOffset, void* pStartAddr)
	{
		m_dwValueOffset = dwValueOffset;
		m_pStartAddr = pStartAddr;
		return *this;
	}

	template<typename T>
	CShmListReverseIterator<T>::CShmListReverseIterator(const CShmListReverseIterator<T>& rhs)
	{
		memcpy(this, &rhs, sizeof(rhs));
	}

	template<typename T>
	const CShmListReverseIterator<T>& CShmListReverseIterator<T>::operator++() const
	{
		m_dwValueOffset = ((SShmListNode<T>*)((char*)m_pStartAddr+m_dwValueOffset))->dwListPreOffset;
		return *this;
	}

	template<typename T>
	CShmListReverseIterator<T>& CShmListReverseIterator<T>::operator++() 
	{
		m_dwValueOffset = ((SShmListNode<T>*)((char*)m_pStartAddr+m_dwValueOffset))->dwListPreOffset;
		return *this;
	}
	template<typename T>
	CShmListReverseIterator<T> CShmListReverseIterator<T>::operator++(int)
	{
		CShmListIterator<T> tmp = *this;

		m_dwValueOffset = ((SShmListNode<T>*)((char*)m_pStartAddr+m_dwValueOffset))->dwListPreOffset;
		return tmp;
	}
	template<typename T>
	const CShmListReverseIterator<T> CShmListReverseIterator<T>::operator++(int) const
	{
		CShmListIterator<T> tmp = *this;
		m_dwValueOffset = ((SShmListNode<T>*)((char*)m_pStartAddr+m_dwValueOffset))->dwListPreOffset;
		return tmp;
	}

	template<typename T>
	CShmListReverseIterator<T>& CShmListReverseIterator<T>::operator--()
	{
		m_dwValueOffset = ((SShmListNode<T>*)((char*)m_pStartAddr+m_dwValueOffset))->dwListNextOffset;
		return *this;
	}

	template<typename T>
	CShmListReverseIterator<T> CShmListReverseIterator<T>::operator--(int)
	{
		CShmListIterator<T> tmp = *this;
		m_dwValueOffset = ((SShmListNode<T>*)((char*)m_pStartAddr+m_dwValueOffset))->dwListNextOffset;
		return tmp;
	}

	template<typename T>
	T & CShmListReverseIterator<T>::operator*()
	{
		return ((SShmListNode<T>*)((char*)m_pStartAddr+m_dwValueOffset))->tVal;
	}

	template<typename T>
	const T* CShmListReverseIterator<T>::operator->() const
	{
		return &(((SShmListNode<T>*)((char*)m_pStartAddr+m_dwValueOffset))->tVal);
	}
	template<typename T>
	T* CShmListReverseIterator<T>::operator->()
	{

		return &(((SShmListNode<T>*)((char*)m_pStartAddr+m_dwValueOffset))->tVal);
	}

	template<typename T>
	T * CShmListReverseIterator<T>::operator&()
	{
		return &(((SShmListNode<T>*)((char*)m_pStartAddr+m_dwValueOffset))->tVal);
	}

	template<typename T>
	bool CShmListReverseIterator<T>::operator==(const CShmListReverseIterator<T>& it) const
	{
		return m_dwValueOffset == it.m_dwValueOffset;
	}

	template<typename T>
	bool CShmListReverseIterator<T>::operator!=(const CShmListReverseIterator<T>& it) const
	{
		return m_dwValueOffset != it.m_dwValueOffset;
	}


};

#endif

