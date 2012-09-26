#ifndef __CANY_VALUE_H__
#define __CANY_VALUE_H__

#include <stdexcept>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <memory.h>
#include <assert.h>
#include <iostream>
#include <deque>
#include <stdint.h>
#include <netinet/in.h>
#include <math.h>
#include <sstream>
#include <iomanip>

using namespace std;

namespace lce
{

#if __BYTE_ORDER == __BIG_ENDIAN
#define ntohll(x)       (x)
#define htonll(x)       (x)
#else
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define ntohll(x)     __bswap_64 (x)
#define htonll(x)     __bswap_64 (x)
#endif
#endif

#define MAX_VALUE_TYPE = 13

struct DType
{
    enum ValueType	//realtype
    {
        Integer1	= 0,		///< tiny int value (1字节)
        Integer2	= 1,		///< small int value (2字节)
        Integer4	= 2,		///< signed integer value(int32)(4字节)
        Integer8	= 3,		///< big signed interger value(int64)(8字节)
        Integer		= Integer8,
        String1		= 4,		///< string value	//1个字节表示长度
        String2		= 5,		///< string value	//2个字节表示长度
        String4		= 6,		///< string value	//4个字节表示长度
        String		= String4,
        Vector		= 7,		///< array value (double list)
        Map			= 8,		///< object value (collection of name/value pairs).
        EXT			= 9,			///< 协议定义描述等信息
        Float       = 10,       /// 浮点 数
        Bool        = 11,       ///布尔
        Null        = 12,        ///空
    };
};

struct EncodeType
{
    enum ENCODETYPE
    {
        NORMAL,		//正常编码
        TYPE1,		//优化编码
    };
};

class CBinary
{
public:
    CBinary()
        :m_bDeepCopy(true)
        ,m_pData(NULL)
        ,m_dwSize(0)
    {}

    CBinary(const char* pData, const size_t dwSize, const bool bDeepCopy=true)
        :m_bDeepCopy(bDeepCopy)
    {
        m_dwSize = dwSize;
        if ( m_bDeepCopy )
        {
            m_pData = new char[dwSize];
            if ( NULL != m_pData )
            {
                memcpy(m_pData, pData, dwSize);
            }
        }
        else
        {
            m_pData = (char*)pData;
        }
    }

    CBinary(const char* pData)
        :m_bDeepCopy(true)
    {
        m_dwSize = strlen(pData);
        m_pData = new char[m_dwSize];
        if ( NULL != m_pData )
        {
            memcpy(m_pData, pData, m_dwSize);
        }
    }

    CBinary(const std::string& sData)
        :m_bDeepCopy(true)
    {
        m_pData = new char[sData.size()];
        if ( NULL != m_pData )
        {
            m_dwSize = sData.size();
            memcpy(m_pData, sData.data(), sData.size());
        }
    }


    ~CBinary()
    {
        this->clear();
    }

    CBinary(const CBinary& rhs)
        :m_bDeepCopy(rhs.m_bDeepCopy)
        ,m_pData(NULL)
        ,m_dwSize(rhs.m_dwSize)
    {
        if (m_bDeepCopy)
        {
            if (NULL != rhs.m_pData)
            {
                m_pData = new char[m_dwSize];
                if ( NULL != m_pData )
                {
                    memcpy(m_pData, rhs.m_pData, m_dwSize);
                }
            }
        }
        else
        {
            m_pData = rhs.m_pData;
        }
    }
    CBinary& operator=(const CBinary& rhs)
    {
        if ( this != &rhs)
        {
            if (m_bDeepCopy && m_pData)
            {
                delete m_pData;
            }
            m_bDeepCopy = rhs.m_bDeepCopy;
            m_dwSize = rhs.m_dwSize;
            if (rhs.m_bDeepCopy)
            {
                if (NULL != rhs.m_pData)
                {
                    m_pData = new char[m_dwSize];
                    assert( NULL != m_pData );
                    memcpy(m_pData, rhs.m_pData, m_dwSize);
                }
            }
            else
            {
                m_pData = rhs.m_pData;
            }
        }

        return *this;
    }

    void assign(const char* pData, const size_t dwSize, const bool bDeepCopy=true)
    {
        if (m_bDeepCopy && m_pData)
        {
            delete m_pData;
            m_pData = NULL;
        }

        if (bDeepCopy)
        {
            m_pData = new char[dwSize];
            assert( NULL != m_pData );
            memcpy(m_pData, pData, dwSize);
        }
        else
        {
            m_pData = (char*)pData;
        }

        m_dwSize = dwSize;
        m_bDeepCopy = bDeepCopy;
    }
    void clear()
    {
        if ( m_bDeepCopy && NULL != m_pData)
        {
            delete m_pData;
        }
        m_pData = NULL;
        m_dwSize = 0;
    }

    size_t size() const
    {
        return m_dwSize;
    }
    const char* data() const
    {
        return m_pData;
    }
protected:
    friend bool operator==(const CBinary& lhs, const CBinary& rhs);
    friend bool operator!=(const CBinary& lhs, const CBinary& rhs);
    friend bool operator<(const CBinary& lhs, const CBinary& rhs);
private:
    bool m_bDeepCopy;
    char* m_pData;
    size_t m_dwSize;
};

inline bool  operator==(const CBinary& lhs, const CBinary& rhs)
{
    return  ( lhs.m_dwSize == rhs.m_dwSize && memcmp(lhs.m_pData, rhs.m_pData, rhs.m_dwSize) == 0 ) ? true : false;
}
inline bool  operator!=(const CBinary& lhs, const CBinary& rhs)
{
    return  ( lhs.m_dwSize != rhs.m_dwSize || memcmp(lhs.m_pData, rhs.m_pData, rhs.m_dwSize) != 0 ) ? true : false;
}
inline bool operator<(const CBinary& lhs, const CBinary& rhs)
{
    bool bOk = false;
    if ( lhs.m_dwSize < rhs.m_dwSize )
    {
        bOk = ( memcmp(lhs.m_pData, rhs.m_pData, lhs.m_dwSize) <= 0 ) ? true : false;
    }
    else
    {
        bOk = ( memcmp(lhs.m_pData, rhs.m_pData, rhs.m_dwSize) < 0 ) ? true : false;
    }

    return  bOk;
}

class CAnyValue
{
public:
    typedef CBinary	BufType;
    typedef CAnyValue this_type;
    typedef std::map<BufType, CAnyValue> MapType;
    typedef std::deque<CAnyValue> VecType;
    typedef std::runtime_error Error;
    typedef std::map<BufType, int> MAPKEY;
    typedef std::vector<BufType> VECKEY;


#pragma pack(1)
    struct SEncodeHead
    {
        SEncodeHead()
        {
            memset(this, 0, sizeof(SEncodeHead));
        }
        unsigned char ucKeyTableType:2;	//0:一个字节表示key value的值； 1: 两个字节表示key value的值； 2：四个字节表示key value的值
        uint32_t dwKeyTableOffset;
        unsigned char szReserve[3];
    };
#pragma pack()


    struct SEncode
    {
        SEncode()
            :ucUsingByteCount(1)
            ,iKeyValue(0)
            ,bNeedTransform(false)
        {}
        unsigned char ucUsingByteCount;		//使用几个字节保存keyvalue信息
        int iKeyValue;						//优化使用的key值
        bool bNeedTransform;			//是否需要转换
        MAPKEY mapKeys;						//string key
        void clear()
        {
            mapKeys.clear();
            bNeedTransform = false;
            iKeyValue = 0;
            ucUsingByteCount = 1;
        }
    };

    struct SDecode
    {
        SDecode()
            :ucUsingByteCount(1)
            ,bNeedTransform(false)
            ,ucValueType(DType::Null)
        {
        };
        unsigned char ucUsingByteCount;		//使用几个字节保存keyvalue信息
        bool bNeedTransform;			//是否需要转换
        unsigned char ucValueType;
        VECKEY vecKeys;
        void clear()
        {
            ucUsingByteCount = 1;
            bNeedTransform = false;
            ucValueType = DType::Null;
            vecKeys.clear();
        }
    };

    union ValueHolder
    {
        uint64_t integer;
        double flValue;
        BufType* buf;
        VecType* vec;
        MapType* map;
    };

    typedef void (*ENCODE_FUNC)(std::string&, const ValueHolder&);
    typedef void (*DECODE_FUNC)(size_t& , const char* , const size_t , this_type& );

public:
    CAnyValue()
        :m_ucType(DType::Null)
        ,m_ucSubType(DType::Null)
        ,m_bInit(false)
        ,m_bHasData(false)
    {
        init();
        m_value.integer = 0;
    }
    CAnyValue(const char cValue)
        :m_ucType(DType::Integer)
        ,m_ucSubType(DType::Integer1)
        ,m_bInit(false)
        ,m_bHasData(cValue==0?false:true)
    {
        init();
        m_value.integer = cValue;
    }

    CAnyValue(const bool bValue)
        :m_ucType(DType::Bool)
        ,m_ucSubType(DType::Bool)
        ,m_bInit(false)
        ,m_bHasData(bValue)
    {
        init();
        m_value.integer = bValue?1:0;
    }

    CAnyValue(const unsigned char ucValue)
        :m_ucType(DType::Integer)
        ,m_ucSubType(DType::Integer1)
        ,m_bInit(false)
        ,m_bHasData(ucValue==0?false:true)
    {
        init();
        m_value.integer = ucValue;
    }
    CAnyValue(const unsigned short wValue)
        :m_ucType(DType::Integer)
        ,m_ucSubType(DType::Integer2)
        ,m_bInit(false)
        ,m_bHasData(wValue==0?false:true)
    {
        init();
        m_value.integer = wValue;
    }
    CAnyValue(const short shValue)
        :m_ucType(DType::Integer)
        ,m_ucSubType(DType::Integer2)
        ,m_bInit(false)
        ,m_bHasData(shValue==0?false:true)
    {
        init();
        m_value.integer = shValue;
    }

#ifdef __x86_64__

    CAnyValue(const long lValue)
        :m_ucType(DType::Integer)
        ,m_ucSubType(DType::Integer8)
        ,m_bInit(false)
        ,m_bHasData(lValue==0?false:true)
    {
        init();
        m_value.integer = lValue;
    }
    CAnyValue(const unsigned long dwValue)
        :m_ucType(DType::Integer)
        ,m_ucSubType(DType::Integer8)
        ,m_bInit(false)
        ,m_bHasData(dwValue==0?false:true)
    {
        init();
        m_value.integer = dwValue;
    }
#else
    CAnyValue(const long lValue)
        :m_ucType(DType::Integer)
        ,m_ucSubType(DType::Integer4)
        ,m_bInit(false)
        ,m_bHasData(lValue==0?false:true)
    {
        init();
        m_value.integer = lValue;
    }
    CAnyValue(const unsigned long dwValue)
        :m_ucType(DType::Integer)
        ,m_ucSubType(DType::Integer4)
        ,m_bInit(false)
        ,m_bHasData(dwValue==0?false:true)
    {
        init();
        m_value.integer = dwValue;
    }

#endif

    CAnyValue(const int iValue)
        :m_ucType(DType::Integer)
        ,m_ucSubType(DType::Integer4)
        ,m_bInit(false)
        ,m_bHasData(iValue==0?false:true)
    {
        init();
        m_value.integer = iValue;
    }
    CAnyValue(const unsigned int uiValue)
        :m_ucType(DType::Integer)
        ,m_ucSubType(DType::Integer4)
        ,m_bInit(false)
        ,m_bHasData(uiValue==0?false:true)
    {
        init();
        m_value.integer = uiValue;
    }

    //64位系统冲突
#ifndef __x86_64__

    CAnyValue(const uint64_t ullValue)
        :m_ucType(DType::Integer)
        ,m_ucSubType(DType::Integer8)
        ,m_bInit(false)
        ,m_bHasData(ullValue==0?false:true)
    {
        init();
        m_value.integer = ullValue;
    }

    CAnyValue(const int64_t llValue)
        :m_ucType(DType::Integer)
        ,m_ucSubType(DType::Integer8)
        ,m_bInit(false)
        ,m_bHasData(llValue==0?false:true)
    {
        init();
        m_value.integer = llValue;
    }

#endif

    CAnyValue(const float flValue)
        :m_ucType(DType::Float)
        ,m_ucSubType(DType::Float)
        ,m_bInit(false)
        ,m_bHasData(flValue==0.0?false:true)
    {
        init();
        m_value.flValue =flValue;

    }


    CAnyValue(const double flValue)
        :m_ucType(DType::Float)
        ,m_ucSubType(DType::Float)
        ,m_bInit(false)
        ,m_bHasData(flValue==0.0?false:true)
    {

        init();
        m_value.flValue =flValue;

    }


    CAnyValue(const char* pszValue)
        :m_ucType(DType::String)
        ,m_ucSubType(DType::String4)
        ,m_bInit(false)
        ,m_bHasData(false)
    {
        init();
        m_value.buf = new BufType(pszValue);
        if ( m_value.buf->size() > 0 )
        {
            m_bHasData = true;
        }


    }

    CAnyValue(const std::string& sValue)
        :m_ucType(DType::String)
        ,m_ucSubType(DType::String4)
        ,m_bInit(false)
        ,m_bHasData(false)
    {
        init();
        m_value.buf  = new BufType(sValue);
        if ( m_value.buf->size() > 0 )
        {
            m_bHasData = true;
        }
    }

    CAnyValue(const CAnyValue& rhs)
        :m_bInit(false)
    {
        init();
        assign(rhs);

    }
    CAnyValue& operator=(const CAnyValue& rhs)
    {
        if ( this != &rhs )
        {
            assign(rhs);
        }

        return *this;
    }

public:

    int asInt() const
    {
        if ( DType::Integer == m_ucType || DType::Bool == m_ucType )	return static_cast<int>(m_value.integer);
        return 0;
    }

    int asUInt() const
    {
        if ( DType::Integer == m_ucType || DType::Bool == m_ucType  )	return static_cast<unsigned int>(m_value.integer);
        return 0;
    }

    bool asBool() const
    {
        if ( DType::Integer == m_ucType || DType::Bool == m_ucType )	return m_value.integer==0 ? false : true;
        return false;
    }

    std::string asString() const
    {
        if ( DType::String == m_ucType && NULL != m_value.buf  )		return std::string(m_value.buf->data(), m_value.buf->size());
        return m_strNull;
    }

    operator uint64_t() const
    {
        if ( DType::Integer == m_ucType || DType::Bool == m_ucType )	return m_value.integer;
        return 0;
    }
    operator int64_t() const
    {
        if ( DType::Integer == m_ucType || DType::Bool == m_ucType  )	return static_cast<int64_t>(m_value.integer);
        return 0;
    }
    operator unsigned char() const
    {
        if ( DType::Integer == m_ucType  || DType::Bool == m_ucType )	return static_cast<unsigned char>(m_value.integer);
        return 0;
    }

    operator unsigned short() const
    {
        if ( DType::Integer == m_ucType || DType::Bool == m_ucType  )	return static_cast<unsigned short>(m_value.integer);
        return 0;
    }

    operator double() const
    {
        if ( DType::Float == m_ucType )	return static_cast<double>(m_value.flValue);
        return 0.0;
    }

    operator short() const
    {
        if ( DType::Integer == m_ucType  || DType::Bool == m_ucType )	return static_cast< short>(m_value.integer);
        return 0;
    }
    operator unsigned long() const
    {
        if ( DType::Integer == m_ucType || DType::Bool == m_ucType )	return static_cast<unsigned long>(m_value.integer);
        return 0;
    }
    operator long() const
    {
        if ( DType::Integer == m_ucType || DType::Bool == m_ucType  )	return static_cast<long>(m_value.integer);
        return 0;
    }
    operator int() const
    {
        if ( DType::Integer == m_ucType || DType::Bool == m_ucType  )	return static_cast<int>(m_value.integer);
        return 0;
    }
    operator unsigned int() const
    {
        if ( DType::Integer == m_ucType || DType::Bool == m_ucType  )	return static_cast<unsigned int>(m_value.integer);
        return 0;
    }
    operator bool() const
    {
        if ( DType::Integer == m_ucType ||DType::Bool == m_ucType )	return m_value.integer==0 ? false : true;
        return false;
    }

    operator std::string() const
    {
        if ( DType::String == m_ucType && NULL != m_value.buf  )		return std::string(m_value.buf->data(), m_value.buf->size());
        return m_strNull;
    }

    const char* data()
    {
        return m_sEncodeData.data();
    }
    size_t size()
    {
        return m_sEncodeData.size();
    }


    const char *valueData() const
    {
        if ( DType::String == m_ucType && NULL != m_value.buf  )		return m_value.buf->data();
        return m_strNull.data();

    }

    const size_t valueSize()	const
    {
        if ( m_ucType == DType::String )
        {
            if ( NULL != m_value.buf )
            {
                return m_value.buf->size();
            }
            else
            {
                assert(false);
            }
        }
        else if ( m_ucType == DType::Vector )
        {
            if ( NULL != m_value.vec )
                return m_value.vec->size();
            else
                assert(false);
        }
        else if ( m_ucType == DType::Map )
        {
            if ( NULL != m_value.map )
                return m_value.map->size();
            else
                assert(false);
        }
        return 0;
    }

public:
    const CAnyValue& operator[](const int iIndex) const
    {
        if ( (DType::Vector==m_ucType) && NULL != m_value.vec )
        {
            if ( iIndex < (int)m_value.vec->size() )
                return (*m_value.vec)[iIndex];
        }
        return m_null;
    }
    CAnyValue& operator[](const int iIndex)
    {
        if ( (DType::Vector==m_ucType) && NULL != m_value.vec )
        {
            if ( iIndex < (int)m_value.vec->size() )
                return (*m_value.vec)[iIndex];
            else throw Error("operator[inx]: index invalid");
        }
        else throw Error("operator[inx]: type invalid");
        //		return m_null;
    }
    const CAnyValue& operator[](const std::string& sName) const
    {
        if ( (DType::Map == m_ucType) && NULL != m_value.map )
        {
            MapType::iterator it = m_value.map->find(sName);
            if ( it != m_value.map->end() )
                return it->second;
        }
        return m_null;
    }

    CAnyValue& operator[](const std::string& sName)
    {
        this->InitAsMap();
        if ( m_value.map == NULL )
        {
            throw Error("anyValue type error: no map type.");
        }
        MapType::iterator it = m_value.map->find(sName);
        if ( it == m_value.map->end() )
        {
            m_bHasData = true;

            if ( sName.size() > 255 )
                it = m_value.map->insert( MapType::value_type(sName.substr(0,255), CAnyValue()) ).first;
            else
                it = m_value.map->insert( MapType::value_type(sName, CAnyValue()) ).first;
        }
        return it->second;
    }

    CAnyValue& operator[](const char* pszName)
    {
        std::string sName(pszName);
        this->InitAsMap();
        if ( m_value.map == NULL )
        {
            throw Error("anyValue type error: no map type.");
        }
        MapType::iterator it = m_value.map->find(sName);
        if ( it == m_value.map->end() )
        {
            m_bHasData = true;
            if ( sName.size() > 255 )
                it = m_value.map->insert( MapType::value_type(sName.substr(0,255), CAnyValue()) ).first;
            else
                it = m_value.map->insert( MapType::value_type(sName, CAnyValue()) ).first;
        }
        return it->second;
    }

    bool hasKey(const std::string& sName) const
    {
        if ( (DType::Map == m_ucType) && NULL != m_value.map )
        {
            MapType::iterator it = m_value.map->find(sName);
            if ( it != m_value.map->end() )
                return true;
        }
        return false;
    }

    //map
    void insert(const std::string& sName, const CAnyValue& oValue)
    {
        this->InitAsMap();
        if ( (DType::Map == m_ucType) && NULL != m_value.map )
        {
            m_bHasData = true;
            if ( sName.size() > 255 )
                (*m_value.map)[sName.substr(0, 255)] = oValue;
            else
                (*m_value.map)[sName] = oValue;
        }
    }

    //vector
    void push_back(const CAnyValue& oValue)
    {
        this->InitAsVector();
        if ( (DType::Vector == m_ucType) && NULL != m_value.vec )
        {
            m_bHasData = true;
            m_value.vec->push_back(oValue);
        }
    }
    void pop_back()
    {
        this->InitAsVector();
        if ( (DType::Vector == m_ucType) && NULL != m_value.vec )
        {
            m_bHasData = true;
            m_value.vec->pop_back();
        }
    }
    void push_front(const CAnyValue& oValue)
    {
        this->InitAsVector();
        if ( (DType::Vector == m_ucType) && NULL != m_value.vec )
        {
            m_bHasData = true;
            m_value.vec->push_front(oValue);
        }
    }
    void pop_front()
    {
        this->InitAsVector();
        if ( (DType::Vector == m_ucType) && NULL != m_value.vec )
        {
            m_bHasData = true;
            m_value.vec->pop_front();
        }
    }

    void erase(const std::string& sName)
    {
        if ( (DType::Map == m_ucType) && NULL != m_value.map )
        {
            m_value.map->erase(sName);
        }
    }

    void clear()
    {

        if ( m_bInit )
        {
            switch(m_ucType)
            {
            case DType::String:
                if ( NULL != m_value.buf )
                {
                    delete m_value.buf;
                    m_value.buf = NULL;
                }
                break;
            case DType::Vector:
                if ( NULL != m_value.vec )
                {
                    delete m_value.vec;
                    m_value.vec = NULL;
                }
                break;
            case DType::Map:
                if ( NULL != m_value.map )
                {
                    delete m_value.map;
                    m_value.map = NULL;
                }
                break;
            }
        }
        else
        {
            assert(false);
        }

        memset(&m_value, 0, sizeof(m_value));
        m_ucType = DType::Null;
        m_ucSubType = DType::Null;
        m_bHasData = false;
    }


    ~CAnyValue()
    {
        this->clear();
    }



    void decode(size_t dwDecodePos,const unsigned char* pData, const size_t dwDataSize)
    {
        this->decode(dwDecodePos,(char*)pData, dwDataSize);
    }


    void decode(size_t dwDecodePos,const char* pData, const size_t dwDataSize)
    {
        if ( dwDataSize < 1 )
        {
            return ;
        }
        unsigned char ucEncodeType = *(unsigned char*)(pData+dwDecodePos);
        dwDecodePos += 1;
        if ( ucEncodeType == EncodeType::NORMAL )
        {
            //正常编解码
            if ( dwDecodePos < dwDataSize )
                this->decodeNM(dwDecodePos,pData, dwDataSize);
        }
        else if ( ucEncodeType == EncodeType::TYPE1 )
        {
            if ( dwDecodePos <  dwDataSize )
                this->decodeOpt(dwDecodePos, pData, dwDataSize);
        }


    }
    void decodeJSON(const char *szBuf,int iSize)
    {
        int dwPos = 0;
        skipSpaces(dwPos,szBuf,iSize);

        if(szBuf[dwPos] == '{')
        {
            dwPos++;
            readObj(dwPos,*this,szBuf,iSize);
        }
        else if(szBuf[dwPos] == '[')
        {
            dwPos++;
            readArray(dwPos,*this,szBuf,iSize);
        }
        else if (szBuf[dwPos] == '"')
        {
            string sValue;
            dwPos++;
            readString(dwPos,sValue,szBuf,iSize);
            dwPos++;

            this->InitAsBuf();
            this->m_value.buf->assign(sValue.data(), sValue.size(), true);
        }
        else if(szBuf[dwPos] == 't')
        {
            if (dwPos+4 > iSize)
            {
                throw runtime_error("not a bool value");
            }

            string sValue;
            sValue.assign(szBuf+dwPos,4);

            if(sValue != "true")
                throw runtime_error("not a bool value");
            this->m_ucType = DType::Bool;
            this->m_value.integer =1;
            dwPos+=4;
        }
        else if(szBuf[dwPos] == 'n')
        {
            if (dwPos+4 > iSize)
            {
                throw runtime_error("not a null value");
            }
            string sValue;
            sValue.assign(szBuf+dwPos,4);

            if(sValue != "null")
                throw runtime_error("not a bool value");
            dwPos+=4;
        }
        else if(szBuf[dwPos] == 'f')
        {
            if (dwPos+5 > iSize)
            {
                throw runtime_error("not a bool value");
            }
            string sValue;
            sValue.assign(szBuf+dwPos,5);

            if(sValue != "false")
                throw runtime_error("not a bool value");
            this->m_ucType = DType::Bool;
            this->m_value.integer = 0;
            dwPos+=5;
        }
        else
        {
            string sValue;
            readNumber(dwPos,sValue,szBuf,iSize);
            if(sValue.find('.')!=string::npos)
            {
                double flValue = atof(sValue.c_str());
                this->m_ucType = DType::Float;
                this->m_value.flValue =flValue;
            }
            else
            {
                uint64_t ddwValue = atoll(sValue.c_str());
                this->m_ucType = DType::Integer;
                this->m_value.integer =ddwValue;
            }

        }
    }

    void encode(const unsigned char ucEncodeType=EncodeType::NORMAL)
    {
        //清除包体数据
        m_sEncodeData.clear();
        m_sEncodeData.append((char*)&ucEncodeType, sizeof(ucEncodeType));

        if ( ucEncodeType == EncodeType::TYPE1 )
        {
            encodeOpt(m_sEncodeData);
        }
        else
        {
            encodeNM(m_sEncodeData);
        }
    }




    void encodeXML(const bool bUtf8=true)
    {
        m_sEncodeData.clear();
        if (bUtf8)
            m_sEncodeData += "<?xml version=\"1.0\" encoding=\"UTF-8\"?><xmldata>";
        else
            m_sEncodeData += "<?xml version=\"1.0\" encoding=\"GB2312\"?><xmldata>";

        encodeXML(m_sEncodeData);
        m_sEncodeData += "</xmldata>";
    }

    void encodeXML(std::string& sBuf)
    {
        switch(m_ucType)
        {
        case DType::Bool:
        {
            if(m_value.integer == 1)
                sBuf += "true";
            else
                sBuf += "false";
        }
        break;
        case DType::Integer:
        {
            sBuf += toStr(m_value.integer);
        }
        break;
        case DType::Float:
        {
            sBuf += toStr(m_value.flValue);
        }
        break;
        case DType::String:
        {
            sBuf += "<![CDATA[";
            sBuf.append(m_value.buf->data(), m_value.buf->size());
            sBuf += "]]>";
        }
        break;
        case DType::Vector:
        {
            for(VecType::iterator it=m_value.vec->begin(); it!=m_value.vec->end(); ++it)
            {
                sBuf += "<li>";
                it->encodeXML(sBuf);
                sBuf += "</li>";
            }
        }
        break;
        case DType::Map:
        {
            for(MapType::iterator it=m_value.map->begin(); it!=m_value.map->end(); ++it)
            {
                sBuf += "<";
                sBuf.append(it->first.data(), it->first.size());
                sBuf += ">";
                it->second.encodeXML(sBuf);
                sBuf += "</";
                sBuf.append(it->first.data(), it->first.size());
                sBuf += ">";
            }
        }
        break;
        case DType::Null:
        {
            sBuf += "null";
        }
        break;
        default:
            break;
        }
    }

    void encodeJSON()
    {

        m_sEncodeData.clear();
        encodeJSON(m_sEncodeData);

    }
    static std::vector<DECODE_FUNC> m_vecDecodeFuncs;
    static const CAnyValue m_null;
    static const std::string m_strNull;

private:

    static void check(const size_t dwPos, const size_t sDataSize)
    {
        if ( dwPos >  sDataSize )
        {
            throw Error("decode data error.");
        }
    }

    template <class T>
    static std::string toStr(const T &t)
    {

        std::stringstream stream;
        stream.precision(16);
        stream<<t;
        return stream.str();
    }

    static std::string textEncodeJSON(const std::string& sSrc)
    {
        std::string sDesc;
        for (std::string::size_type i=0; i<sSrc.size(); i++)
        {
            if (sSrc[i] == '\"')
            {
                sDesc += "\\\"";
            }
            else if (sSrc[i] == '\r')
            {
                sDesc += "\\r";
            }
            else if (sSrc[i] == '\n')
            {
                sDesc += "\\n";
            }
            else if (sSrc[i] == '\\')
            {
                sDesc += "\\\\";
            }
            else if (sSrc[i] == '/')
            {
                sDesc += "\\/";
            }
            else if (sSrc[i] == '\b')
            {
                sDesc += "\\b";
            }
            else if (sSrc[i] == '\f')
            {
                sDesc += "\\f";
            }
            else if (sSrc[i] == '\t')
            {
                sDesc += "\\t";
            }
            else
            {
                if(isControlCharacter(sSrc[i]))
                {
                    std::ostringstream oss;
                    oss << "\\u" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << static_cast<int>(sSrc[i]);
                    sDesc += oss.str();
                }
                else
                {
                    sDesc += sSrc[i];
                }

            }
        }
        return sDesc;
    }

    static inline bool isControlCharacter(char ch)
    {
        return ch > 0 && ch <= 0x1F;
    }

public:
    void encodeJSON(std::string& sBuf)
    {
        switch(m_ucType)
        {

        case DType::Bool:
        {
            if (m_value.integer == 1)
                sBuf += "true";
            else
                sBuf += "false";
        }
        break;

        case DType::Integer:
        {
            sBuf += toStr(m_value.integer);
        }
        break;
        case DType::Float:
        {
            sBuf += toStr(m_value.flValue);
        }
        break;
        case DType::String:
        {
            sBuf += "\"";
            sBuf += textEncodeJSON(string(m_value.buf->data(), m_value.buf->size()));
            sBuf += "\"";
        }
        break;
        case DType::Vector:
        {
            bool bFirst = true;
            sBuf += "[";
            for(VecType::iterator it=m_value.vec->begin(); it!=m_value.vec->end(); ++it)
            {
                if ( !bFirst )
                {
                    sBuf += ",";
                }
                else
                {
                    bFirst = false;
                }
                it->encodeJSON(sBuf);
            }
            sBuf += "]";
        }
        break;
        case DType::Map:
        {

            sBuf += "{";
            bool bFirst = true;
            for(MapType::iterator it=m_value.map->begin(); it!=m_value.map->end(); ++it)
            {
                if ( bFirst )
                {
                    sBuf += "\"";
                    bFirst = false;
                }
                else
                {
                    sBuf += ",\"";
                }
                sBuf += textEncodeJSON(string(it->first.data(), it->first.size()));
                sBuf += "\":";
                it->second.encodeJSON(sBuf);
            }
            sBuf += "}";
        }
        break;
        case DType::Null:
        {
            sBuf += "null";
        }
        break;
        default:
        {
            sBuf += "\"\"";
        }
        break;
        }
    }

private:

    void decode_type1(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, const SDecode& stDecode)
    {
        if ( stDecode.bNeedTransform )
        {
            m_ucSubType = stDecode.ucValueType;
            *(bool*)&stDecode.bNeedTransform = false;
        }
        else
        {
            check(dwDecodePos+1, dwDataSize);
            m_ucSubType = *(pData+dwDecodePos);
            dwDecodePos++;
        }
        switch(m_ucSubType)
        {
        case DType::Bool:
        {
            m_ucType = DType::Bool;
            check(dwDecodePos+1, dwDataSize);
            m_value.integer = *(unsigned char*)(pData+dwDecodePos);
            ++dwDecodePos;
        }
        break;
        case DType::Integer1:
        {
            m_ucType = DType::Integer;
            check(dwDecodePos+1, dwDataSize);
            m_value.integer = *(unsigned char*)(pData+dwDecodePos);
            ++dwDecodePos;
        }
        break;
        case DType::Integer2:
        {
            m_ucType = DType::Integer;
            check(dwDecodePos+2, dwDataSize);
            m_value.integer = ntohs(*(unsigned short*)(pData+dwDecodePos));
            dwDecodePos += 2;
        }
        break;
        case DType::Integer4:
        {
            m_ucType = DType::Integer;
            check(dwDecodePos+4, dwDataSize);
            m_value.integer = ntohl(*(uint32_t*)(pData+dwDecodePos));
            dwDecodePos += 4;
        }
        break;
        case DType::Integer8:
        {
            m_ucType = DType::Integer;
            check(dwDecodePos+8, dwDataSize);
            //				m_value.integer = (*(long long*)(pData+dwDecodePos));
            m_value.integer = ntohll(*(uint64_t*)(pData+dwDecodePos));
            dwDecodePos += 8;
        }
        break;
        case DType::Float:
        {
            m_ucType = DType::Float;
            check(dwDecodePos+1, dwDataSize);
            unsigned char ucStrLen = *(unsigned char*)(pData+dwDecodePos);
            ++dwDecodePos;
            check(dwDecodePos+ucStrLen, dwDataSize);

            std::stringstream stream;
            stream.write(pData+dwDecodePos,ucStrLen);
            stream.precision(16);
            stream>>m_value.flValue;
            dwDecodePos += ucStrLen;

        }
        break;
        case DType::String1:
        {
            this->InitAsBuf();
            check(dwDecodePos+1, dwDataSize);
            unsigned char ucStrLen = *(unsigned char*)(pData+dwDecodePos);
            ++dwDecodePos;
            check(dwDecodePos+ucStrLen, dwDataSize);
            m_value.buf->assign(pData+dwDecodePos, ucStrLen, true);
            dwDecodePos += ucStrLen;
        }
        break;
        case DType::String2:
        {
            this->InitAsBuf();
            check(dwDecodePos+2, dwDataSize);
            unsigned short wStrLen = ntohs(*(unsigned short*)(pData+dwDecodePos));
            dwDecodePos += 2;
            check(dwDecodePos+wStrLen, dwDataSize);
            m_value.buf->assign(pData+dwDecodePos, wStrLen, true);
            dwDecodePos += wStrLen;
        }
        break;
        case DType::String4:
        {
            this->InitAsBuf();
            check(dwDecodePos+4, dwDataSize);
            uint32_t dwStrLen = ntohl(*(uint32_t*)(pData+dwDecodePos));
            dwDecodePos += 4;
            check(dwDecodePos+dwStrLen, dwDataSize);
            m_value.buf->assign(pData+dwDecodePos, dwStrLen, true);
            dwDecodePos += dwStrLen;
        }
        break;
        case DType::Vector:
        {
            this->InitAsVector();
            check(dwDecodePos+4, dwDataSize);
            uint32_t dwSize = ntohl(*(uint32_t*)(pData+dwDecodePos));
            dwDecodePos += 4;
            while ( dwSize > 0 )
            {
                --dwSize;
                CAnyValue value;
                value.decode_type1(dwDecodePos, pData, dwDataSize, stDecode);
                m_value.vec->push_back(value);

            }
        }
        break;
        case DType::Map:
        {
            this->InitAsMap();
            check(dwDecodePos+4, dwDataSize);
            uint32_t dwSize = ntohl(*(uint32_t*)(pData+dwDecodePos));
            dwDecodePos += 4;
            while ( dwSize > 0 )
            {
                --dwSize;
                uint32_t dwKeyType = 0;
                if ( 1 == stDecode.ucUsingByteCount )
                {
                    check(dwDecodePos+1, dwDataSize);
                    dwKeyType = *(unsigned char*)(pData+dwDecodePos);
                    ++dwDecodePos;
                }
                else if( 2==stDecode.ucUsingByteCount )
                {
                    check(dwDecodePos+2, dwDataSize);
                    dwKeyType = ntohs(*(unsigned short*)(pData+dwDecodePos));
                    dwDecodePos += 2;
                }
                else if ( 4==stDecode.ucUsingByteCount )
                {
                    check(dwDecodePos+4, dwDataSize);
                    dwKeyType = ntohl(*(uint32_t*)(pData+dwDecodePos));
                    dwDecodePos += 4;
                }
                else
                {
                    assert(false);
                    throw Error("stDecode.ucUsingByteCount error.");
                }
                *(unsigned char*)&stDecode.ucValueType = static_cast<unsigned char>(dwKeyType%13);
                uint32_t dwKeyValue = dwKeyType/13;
                assert(dwKeyValue < stDecode.vecKeys.size());
                if ( dwKeyValue >= stDecode.vecKeys.size() )
                    throw Error("dwKeyValue >= stDecode.vecKeys.size()");

                BufType sName(stDecode.vecKeys[dwKeyType/13]);

                *(bool*)&stDecode.bNeedTransform = true;
                CAnyValue value;
                value.decode_type1(dwDecodePos, pData, dwDataSize, stDecode);
                m_value.map->insert( MapType::value_type(sName, value) );
                *(bool*)&stDecode.bNeedTransform = false;
            }

        }
        break;
        case DType::EXT:
        {
            //check(dwDecodePos+1, dwDataSize);
            //unsigned char ucType = *(unsigned char*)(pData+dwDecodePos);
            //++dwDecodePos;
        }
        break;
        case DType::Null:
        {

        }
        break;
        default:
            break;
        }
    }

    static void decode_bool(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj)
    {
        thisobj.m_ucType = DType::Bool;
        check(dwDecodePos+1, dwDataSize);
        thisobj.m_value.integer = (unsigned char)*(unsigned char*)(pData+dwDecodePos);
        ++dwDecodePos;

    }

    static void decode_integer1(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj)
    {
        thisobj.m_ucType = DType::Integer;
        check(dwDecodePos+1, dwDataSize);
        thisobj.m_value.integer = (unsigned char)*(unsigned char*)(pData+dwDecodePos);
        ++dwDecodePos;

    }
    static void decode_integer2(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj)
    {
        thisobj.m_ucType = DType::Integer;
        check(dwDecodePos+2, dwDataSize);
        thisobj.m_value.integer = ntohs(*(unsigned short*)(pData+dwDecodePos));
        dwDecodePos += 2;
    }
    static void decode_integer4(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj)
    {
        thisobj.m_ucType = DType::Integer;
        check(dwDecodePos+4, dwDataSize);
        thisobj.m_value.integer = ntohl(*(uint32_t*)(pData+dwDecodePos));
        dwDecodePos += 4;
    }
    static void decode_integer8(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj)
    {
        thisobj.m_ucType = DType::Integer;
        check(dwDecodePos+8, dwDataSize);
        //		thisobj.m_value.integer = (*(long long*)(pData+dwDecodePos));

        thisobj.m_value.integer = ntohll(*(uint64_t*)(pData+dwDecodePos));
        dwDecodePos += 8;
    }
    static void decode_string1(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj)
    {
        thisobj.InitAsBuf();
        check(dwDecodePos+1, dwDataSize);
        unsigned char ucStrLen = *(unsigned char*)(pData+dwDecodePos);
        ++dwDecodePos;
        check(dwDecodePos+ucStrLen, dwDataSize);
        thisobj.m_value.buf->assign(pData+dwDecodePos, ucStrLen, true);
        dwDecodePos += ucStrLen;
    }
    static void decode_string2(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj)
    {
        thisobj.InitAsBuf();
        check(dwDecodePos+2, dwDataSize);
        unsigned short wStrLen = ntohs(*(unsigned short*)(pData+dwDecodePos));
        dwDecodePos += 2;
        check(dwDecodePos+wStrLen, dwDataSize);
        thisobj.m_value.buf->assign(pData+dwDecodePos, wStrLen, true);
        dwDecodePos += wStrLen;
    }
    static void decode_string4(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj)
    {
        thisobj.InitAsBuf();
        check(dwDecodePos+4, dwDataSize);
        uint32_t dwStrLen = ntohl(*(uint32_t*)(pData+dwDecodePos));
        dwDecodePos += 4;
        check(dwDecodePos+dwStrLen, dwDataSize);
        thisobj.m_value.buf->assign(pData+dwDecodePos, dwStrLen, true);
        dwDecodePos += dwStrLen;
    }
    static void decode_vector(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj)
    {
        thisobj.InitAsVector();
        check(dwDecodePos+4, dwDataSize);
        uint32_t dwSize = ntohl(*(uint32_t*)(pData+dwDecodePos));
        dwDecodePos += 4;
        while ( dwSize > 0 )
        {

            --dwSize;
            CAnyValue value;
            value.decodeNM(dwDecodePos, pData, dwDataSize);
            thisobj.m_value.vec->push_back(value);

        }
    }
    static void decode_map(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj)
    {
        thisobj.InitAsMap();
        check(dwDecodePos+4, dwDataSize);
        uint32_t dwSize = ntohl(*(uint32_t*)(pData+dwDecodePos));
        dwDecodePos += 4;
        while ( dwSize > 0 )
        {
            --dwSize;
            check(dwDecodePos+1, dwDataSize);
            unsigned char ucNameLen =  *(unsigned char*)(pData+dwDecodePos);
            ++dwDecodePos;
            check(dwDecodePos+ucNameLen, dwDataSize);
            BufType sName(pData+dwDecodePos, ucNameLen, true);
            dwDecodePos += ucNameLen;
//				cout << std::string(sName.data(), sName.size()) << endl;
            if ( dwDataSize > dwDecodePos )
            {
                CAnyValue value;
                value.decodeNM(dwDecodePos, pData, dwDataSize);
                thisobj.m_value.map->insert( MapType::value_type(sName, value) );
            }
        }
    }

    static void decode_null(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj)
    {
        thisobj.m_ucType = DType::Null;
    }
    static void decode_ext(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj)
    {

    }
    static void decode_float(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj)
    {


        thisobj.m_ucType = DType::Float;
        check(dwDecodePos+1, dwDataSize);
        unsigned char ucStrLen = *(unsigned char*)(pData+dwDecodePos);
        ++dwDecodePos;
        check(dwDecodePos+ucStrLen, dwDataSize);

        std::stringstream stream;
        stream.write(pData+dwDecodePos,ucStrLen);
        stream.precision(16);
        stream>>thisobj.m_value.flValue;
        dwDecodePos += ucStrLen;
    }

    void decodeNM(size_t& dwDecodePos, const char* pData, const size_t dwDataSize)
    {
        static DECODE_FUNC arDecodeFunc[13] = {&decode_integer1, &decode_integer2, &decode_integer4, &decode_integer8,&decode_string1,
                                               &decode_string2, &decode_string4, &decode_vector, &decode_map,  &decode_ext, &decode_float,&decode_bool,&decode_null
                                              };
        check(dwDecodePos+1, dwDataSize);
        m_ucSubType = *(pData+dwDecodePos);
        dwDecodePos++;

        if ( m_ucSubType< 13 )
        {
            arDecodeFunc[m_ucSubType](dwDecodePos, pData, dwDataSize, *this);
        }
    }


    void decodeOpt(size_t &dwDecodePos,const char* pData, const size_t dwDataSize)
    {

        m_stDecode.clear();
        if ( dwDataSize < dwDecodePos+sizeof(SEncodeHead) )
        {
            assert(false);
            throw Error("decode type1 error.");
        }
        //优化编解码
        //read encode head
        SEncodeHead stEncodeHead;
        memcpy(&stEncodeHead, pData+dwDecodePos, sizeof(stEncodeHead));
        stEncodeHead.dwKeyTableOffset = ntohl(stEncodeHead.dwKeyTableOffset);

        dwDecodePos += sizeof(SEncodeHead);
        decodeKeyTable(stEncodeHead, pData+stEncodeHead.dwKeyTableOffset, dwDataSize-stEncodeHead.dwKeyTableOffset);

        if ( dwDecodePos <  dwDataSize )
            this->decode_type1(dwDecodePos, pData, stEncodeHead.dwKeyTableOffset, m_stDecode);

    }

    void decodeKeyTable(const SEncodeHead stEncodeHead, const char* pData, const size_t dwDataSize)
    {
        //decode key table
        size_t dwPos = 0;
        unsigned short wKeyTableSize = ntohs(*(unsigned short*)(pData+dwPos));

        dwPos += 2;
        m_stDecode.vecKeys.resize(wKeyTableSize+1);
        if ( 0 == stEncodeHead.ucKeyTableType )
        {
            for ( uint32_t i=0; i<wKeyTableSize; ++i )
            {
                check(dwPos+2, dwDataSize);
                unsigned char ucKeyValue = *(pData+dwPos);
                dwPos += 1;
                unsigned char ucKeyNameLen = *(pData+dwPos);
                dwPos += 1;
                check(dwPos+ucKeyNameLen, dwDataSize);
                std::string sName(pData+dwPos, ucKeyNameLen);
                dwPos += ucKeyNameLen;

                assert( ucKeyValue < m_stDecode.vecKeys.size());
                if ( ucKeyValue >= m_stDecode.vecKeys.size() )
                    throw Error("ucKeyValue >= m_stDecode.vecKeys.size()");
                m_stDecode.vecKeys[ucKeyValue] = sName;
            }
        }
        else if ( 1 == stEncodeHead.ucKeyTableType )
        {
            for ( uint32_t i=0; i<wKeyTableSize; ++i )
            {
                check(dwPos+3, dwDataSize);
                unsigned short wKeyValue = *(unsigned short*)(pData+dwPos);
                dwPos += 2;
                unsigned char ucKeyNameLen = *(pData+dwPos);
                dwPos += 1;
                check(dwPos+ucKeyNameLen, dwDataSize);
                std::string sName(pData+dwPos, ucKeyNameLen);
                dwPos += ucKeyNameLen;

                assert( wKeyValue < m_stDecode.vecKeys.size());
                if ( wKeyValue >= m_stDecode.vecKeys.size() )
                    throw Error("wKeyValue >= m_stDecode.vecKeys.size()");

                m_stDecode.vecKeys[wKeyValue] = sName;
            }
        }
        else
        {
            assert(false);
            throw Error("stEncodeHead.ucKeyTableType error: no suport.");
        }
    }



    static inline std::string unicodeToUTF8(unsigned int cp)
    {
        std::string result;

        if (cp <= 0x7f)
        {
            result.resize(1);
            result[0] = static_cast<char>(cp);
        }
        else if (cp <= 0x7FF)
        {
            result.resize(2);
            result[1] = static_cast<char>(0x80 | (0x3f & cp));
            result[0] = static_cast<char>(0xC0 | (0x1f & (cp >> 6)));
        }
        else if (cp <= 0xFFFF)
        {
            result.resize(3);
            result[2] = static_cast<char>(0x80 | (0x3f & cp));
            result[1] = 0x80 | static_cast<char>((0x3f & (cp >> 6)));
            result[0] = 0xE0 | static_cast<char>((0xf & (cp >> 12)));
        }
        else if (cp <= 0x10FFFF)
        {
            result.resize(4);
            result[3] = static_cast<char>(0x80 | (0x3f & cp));
            result[2] = static_cast<char>(0x80 | (0x3f & (cp >> 6)));
            result[1] = static_cast<char>(0x80 | (0x3f & (cp >> 12)));
            result[0] = static_cast<char>(0xF0 | (0x7 & (cp >> 18)));
        }

        return result;
    }

    static inline void readUnicode( int &dwPos,const char* szBuf,int iSize, unsigned int &unicode )
    {

        unicode = 0;
        for ( int index =0; index < 4; ++index )
        {
            dwPos++;
            if( dwPos >= iSize)
            {
                throw runtime_error( "over range");
            }
            char c = szBuf[dwPos];
            unicode *= 16;
            if ( c >= '0'  &&  c <= '9' )
                unicode += c - '0';
            else if ( c >= 'a'  &&  c <= 'f' )
                unicode += c - 'a' + 10;
            else if ( c >= 'A'  &&  c <= 'F' )
                unicode += c - 'A' + 10;
            else
            {
                string sOut;
                sOut.append(szBuf+dwPos,min(100,iSize-dwPos));
                cout<<sOut<<endl;
                throw runtime_error( "Bad unicode escape sequence in string: hexadecimal digit expected.");
            }


        }

    }


    static void skipSpaces(int &dwPos,const char* szBuf,int iSize)
    {
        while(dwPos < iSize)
        {
            if (szBuf[dwPos] == ' ' ||szBuf[dwPos] == '\t' ||szBuf[dwPos] == '\r' ||szBuf[dwPos] == '\n')
            {
                dwPos++;
            }
            else
            {
                break;
            }
        }
    }

    static void readString(int &dwPos,string &sValue,const char *szBuf,int iSize)
    {
        int dwFlag = 0;
        while(dwPos < iSize)
        {
            if(szBuf[dwPos] == '\\' && szBuf[dwPos+1] == '"')
            {
                sValue.append("\"");
                dwPos++;
            }
            else if(szBuf[dwPos] == '\\' && szBuf[dwPos+1] == 't')
            {
                sValue.append("\t");
                dwPos++;
            }
            else if(szBuf[dwPos] == '\\' && szBuf[dwPos+1] == 'b')
            {
                sValue.append("\b");
                dwPos++;
            }
            else if(szBuf[dwPos] == '\\' && szBuf[dwPos+1] == 'f')
            {
                sValue.append("\f");
                dwPos++;
            }
            else if(szBuf[dwPos] == '\\' && szBuf[dwPos+1] == 'n')
            {
                sValue.append("\n");
                dwPos++;
            }
            else if(szBuf[dwPos] == '\\' && szBuf[dwPos+1] == 'r')
            {
                sValue.append("\r");
                dwPos++;
            }
            else if(szBuf[dwPos] == '\\' && szBuf[dwPos+1] == '\\')
            {
                sValue.append("\\");
                dwPos++;
            }
            else if(szBuf[dwPos] == '\\' && szBuf[dwPos+1] == '/')
            {
                sValue.append("/");
                dwPos++;
            }
            else if(szBuf[dwPos] == '\\' && szBuf[dwPos+1] == 'u')
            {
                dwPos++;
                uint32_t unicode = 0;
                readUnicode(dwPos,szBuf,iSize,unicode);
                sValue.append(unicodeToUTF8(unicode));

            }
            else if(szBuf[dwPos] == '"')
            {
                dwFlag = 1;
                break;
            }
            else
            {
                sValue.append(szBuf+dwPos,1);
            }
            dwPos++;

        }

        if(dwFlag == 0)
        {
            string sOut;
            sOut.append(szBuf+dwPos,min(100,iSize-dwPos));
            cout<<sOut<<endl;
            throw runtime_error("read string error");
        }
    }

    static void readNumber(int &dwPos,string &sValue,const char *szBuf,int iSize)
    {
        while(dwPos < iSize)
        {
            sValue.append(szBuf+dwPos,1);
            dwPos++;

            if(szBuf[dwPos] == ' ' ||
                    szBuf[dwPos] == ',' ||
                    szBuf[dwPos] == '}' ||
                    szBuf[dwPos] == ']' ||
                    szBuf[dwPos] == '\r'||
                    szBuf[dwPos] == '\n'||
                    szBuf[dwPos] == '\t'||
                    szBuf[dwPos] == '\b'||
                    szBuf[dwPos] == '\f'  )
            {
                break;
            }
        }


    }

    static void readArray(int &dwPos,CAnyValue &oValue,const char *szBuf,int iSize)
    {
        oValue.InitAsVector();

        while (dwPos < iSize )
        {
            skipSpaces(dwPos,szBuf,iSize);
            if(szBuf[dwPos] == ']')
            {
                dwPos++;
                break;
            }
            else if(szBuf[dwPos] == '{')
            {
                dwPos++;
                CAnyValue objValue;
                readObj(dwPos,objValue,szBuf,iSize);
                oValue.push_back(objValue);
            }
            else if (szBuf[dwPos] == '[')
            {
                dwPos++;
                CAnyValue arrayValue;
                readArray(dwPos,arrayValue,szBuf,iSize);
                oValue.push_back(arrayValue);
            }
            else if (szBuf[dwPos] == '"')
            {
                string sValue;
                dwPos++;
                readString(dwPos,sValue,szBuf,iSize);
                dwPos++;
                oValue.push_back(sValue);
            }
            else if(szBuf[dwPos] == 't')
            {
                if (dwPos+4 >= iSize)
                {
                    throw runtime_error("not a bool value");
                }

                string sValue;
                sValue.assign(szBuf+dwPos,4);

                if(sValue != "true")
                    throw runtime_error("not a bool value");
                oValue.push_back(true);
                dwPos+=4;
            }
            else if(szBuf[dwPos] == 'n')
            {
                if (dwPos+4 >= iSize)
                {
                    throw runtime_error("not a null value");
                }
                string sValue;
                sValue.assign(szBuf+dwPos,4);

                if(sValue != "null")
                    throw runtime_error("not a bool value");

                oValue.push_back(CAnyValue::m_null);
                dwPos+=4;
            }
            else if(szBuf[dwPos] == 'f')
            {
                if (dwPos+5 >= iSize)
                {
                    throw runtime_error("not a bool value");
                }
                string sValue;
                sValue.assign(szBuf+dwPos,5);

                if(sValue != "false")
                    throw runtime_error("not a bool value");
                oValue.push_back(false);
                dwPos+=5;
            }
            else
            {
                string sValue;
                readNumber(dwPos,sValue,szBuf,iSize);
                if(sValue.find('.')!=string::npos)
                {
                    double flValue = atof(sValue.c_str());
                    oValue.push_back(flValue);
                }
                else
                {
                    uint64_t ddwValue = atoll(sValue.c_str());
                    oValue.push_back(ddwValue);
                }

            }
            skipSpaces(dwPos,szBuf,iSize);
            if(szBuf[dwPos] == ']')
            {
                dwPos++;
                break;
            }
            else if (szBuf[dwPos] == ',')
            {
                dwPos++;
            }
            else
            {
                string sOut;
                sOut.append(szBuf+dwPos,min(100,iSize-dwPos));
                cout<<sOut<<endl;
                throw runtime_error("expect ','");
            }
        }

    }


    static void readObj(int &dwPos,CAnyValue &oValue,const char *szBuf,int iSize)
    {
        oValue.InitAsMap();

        while(dwPos < iSize)
        {
            skipSpaces(dwPos,szBuf,iSize);
            if(szBuf[dwPos] == '}')
            {
                dwPos++;
                break;
            }
            if(szBuf[dwPos] == '"')
            {
                string sKey;
                dwPos++;
                readString(dwPos,sKey,szBuf,iSize);
                dwPos++;
                skipSpaces(dwPos,szBuf,iSize);

                if(szBuf[dwPos] != ':')
                {
                    string sOut;
                    sOut.append(szBuf+dwPos,min(100,iSize-dwPos));
                    cout<<sOut<<endl;
                    throw runtime_error("expect ':'");
                }


                dwPos++;//skip ":"
                skipSpaces(dwPos,szBuf,iSize);
                if (szBuf[dwPos] == '"')
                {
                    dwPos++;
                    string sValue;
                    readString(dwPos,sValue,szBuf,iSize);
                    dwPos++;
                    oValue[sKey] = sValue;
                }
                else if(szBuf[dwPos] == '{')
                {
                    dwPos++;
                    CAnyValue objValue;
                    readObj(dwPos,objValue,szBuf,iSize);
                    oValue[sKey] = objValue;
                }
                else if(szBuf[dwPos] == '[')
                {
                    dwPos++;
                    CAnyValue arrayValue;
                    readArray(dwPos,arrayValue,szBuf,iSize);
                    oValue[sKey] = arrayValue;
                }
                else if(szBuf[dwPos] == 't')
                {

                    if (dwPos+4 >= iSize)
                    {
                        throw runtime_error("not a bool value");
                    }

                    string sValue;
                    sValue.assign(szBuf+dwPos,4);

                    if(sValue != "true")
                    {
                        string sOut;
                        sOut.append(szBuf+dwPos,min(100,iSize-dwPos));
                        cout<<sOut<<endl;
                        throw runtime_error("not a bool value");
                    }

                    oValue[sKey]=true;
                    dwPos+=4;
                }
                else if(szBuf[dwPos] == 'n')
                {
                    if (dwPos+4 >= iSize)
                    {
                        throw runtime_error("not a null value");
                    }
                    string sValue;
                    sValue.assign(szBuf+dwPos,4);

                    if(sValue != "null")
                        throw runtime_error("not a null value");
                    oValue[sKey];
                    dwPos+=4;
                }
                else if(szBuf[dwPos] == 'f')
                {
                    if (dwPos+5 >= iSize)
                    {
                        throw runtime_error("not a bool value");
                    }
                    string sValue;
                    sValue.assign(szBuf+dwPos,5);

                    if(sValue != "false")
                        throw runtime_error("not a bool value");
                    oValue[sKey]=false;
                    dwPos+=5;
                }
                else
                {
                    string sValue;
                    readNumber(dwPos,sValue,szBuf,iSize);
                    if(sValue.find('.')!=string::npos)
                    {
                        double flValue = atof(sValue.c_str());
                        oValue[sKey]=flValue;
                    }
                    else
                    {
                        uint64_t ddwValue = atoll(sValue.c_str());
                        oValue[sKey]=ddwValue;
                    }

                }

                skipSpaces(dwPos,szBuf,iSize);

                if(szBuf[dwPos] == '}')
                {
                    dwPos++;
                    break;
                }
                else if (szBuf[dwPos] == ',')
                {
                    dwPos++;
                }
                else
                {
                    string sOut;
                    sOut.append(szBuf+dwPos,min(100,iSize-dwPos));
                    cout<<sOut<<endl;
                    throw runtime_error("expect ','");
                }

            }
            else
            {
                string sOut;
                sOut.append(szBuf+dwPos,min(100,iSize-dwPos));
                cout<<sOut<<endl;
                throw runtime_error("expect '\"'");
            }
        }

    }

    void encode_type1(std::string& sBuf, SEncode& stEncode)
    {
        switch(m_ucType)
        {

        case DType::Bool:
        {
            writeKeyType(sBuf, stEncode, DType::Bool);
            sBuf.push_back((char)m_value.integer);

        }
        break;
        case DType::Integer:
        {

            if ( m_value.integer <= 0xFF )
            {
                writeKeyType(sBuf, stEncode, DType::Integer1);
                sBuf.push_back((char)m_value.integer);
            }

            else if ( m_value.integer <= 0xFFFF )
            {
                writeKeyType(sBuf, stEncode, DType::Integer2);
                unsigned short wTmp = htons(static_cast<unsigned short>(m_value.integer));
                sBuf.append(reinterpret_cast<char*>(&wTmp),sizeof(wTmp));
            }
            else if ( m_value.integer <= 0xFFFFFFFF )
            {
                writeKeyType(sBuf, stEncode, DType::Integer4);
                uint32_t dwTmp = htonl(static_cast<uint32_t>(m_value.integer));
                sBuf.append(reinterpret_cast<char*>(&dwTmp),sizeof(dwTmp));
            }
            else
            {
                writeKeyType(sBuf, stEncode, DType::Integer8);
                //					sBuf.append(reinterpret_cast<char*>(&m_value.integer),sizeof(m_value.integer));
                uint64_t ui64Tmp = htonll(m_value.integer);
                sBuf.append(reinterpret_cast<char*>(&ui64Tmp),sizeof(ui64Tmp));
            }
        }
        break;

        case DType::Float:
        {
            writeKeyType(sBuf, stEncode, DType::Float);
            string sValue = toStr(m_value.flValue);
            sBuf.push_back(static_cast<char>(sValue.size()));
            sBuf.append(sValue.data(),sValue.size());
        }
        break;
        case DType::String:
        {
            if ( m_value.buf->size() <= 0xFF )
            {
                writeKeyType(sBuf, stEncode, DType::String1);
                sBuf.push_back(static_cast<char>(m_value.buf->size()));
            }
            else if ( m_value.buf->size() <= 0xFFFF )
            {
                writeKeyType(sBuf, stEncode, DType::String2);
                unsigned short wSize = htons(static_cast<unsigned short>(m_value.buf->size()));
                sBuf.append(reinterpret_cast<char*>(&wSize), sizeof(wSize));
            }
            else
            {
                writeKeyType(sBuf, stEncode, DType::String4);
                uint32_t dwSize = htonl(static_cast<uint32_t>(m_value.buf->size()));
                sBuf.append(reinterpret_cast<char*>(&dwSize), sizeof(dwSize));
            }
            sBuf.append(m_value.buf->data(), m_value.buf->size());
        }
        break;
        case DType::Vector:
        {
            writeKeyType(sBuf, stEncode, DType::Vector);
            uint32_t dwSize = htonl(static_cast<uint32_t>(m_value.vec->size()));
            sBuf.append(reinterpret_cast<char*>(&dwSize), sizeof(dwSize));
            for(VecType::iterator it=m_value.vec->begin(); it!=m_value.vec->end(); ++it)
                it->encode_type1(sBuf,stEncode);
        }
        break;
        case DType::Map:
        {
            writeKeyType(sBuf, stEncode, DType::Map);
            uint32_t dwSize = 0;//htonl(static_cast<unsigned long>(m_value.map->size()));
            size_t dwSizePos = sBuf.size();
            sBuf.append(reinterpret_cast<char*>(&dwSize), sizeof(dwSize));
            for(MapType::iterator it=m_value.map->begin(); it!=m_value.map->end(); ++it)
            {
                ++dwSize;
                stEncode.iKeyValue = stEncode.mapKeys.insert( MAPKEY::value_type( std::string(it->first.data(), it->first.size()), (int)stEncode.mapKeys.size()) ).first->second;
                stEncode.bNeedTransform = true;
                it->second.encode_type1(sBuf, stEncode);
                stEncode.bNeedTransform = false;

            }
            dwSize = htonl(dwSize);
            memcpy((char*)sBuf.data()+dwSizePos, reinterpret_cast<char*>(&dwSize), sizeof(dwSize));
        }
        break;
        case DType::Null:
        {
            writeKeyType(sBuf, stEncode, DType::Null);
        }
        default:
            break;
        }
    }

public:
    void encodeOpt(std::string& sBuf)
    {

        //优化编解码
        m_stEncode.clear();
        //encode type1 信息
        SEncodeHead stEncodeHead;
        size_t dwEncodeHeadPos = sBuf.size();
        sBuf.append(reinterpret_cast<char*>(&stEncodeHead), sizeof(stEncodeHead));

        //编码
        this->encode_type1(sBuf, m_stEncode);

        //write key table;
        stEncodeHead.dwKeyTableOffset = htonl((uint32_t)sBuf.size());

        //table size
        assert(m_stEncode.mapKeys.size() <= 0xFFFF);
        unsigned short wKeyTableSize = htons((unsigned short)m_stEncode.mapKeys.size());
        sBuf.append((char*)&wKeyTableSize, sizeof(wKeyTableSize));

        if ( m_stEncode.mapKeys.size() <= 0xFF )
        {
            stEncodeHead.ucKeyTableType = 0;
            for ( MAPKEY::const_iterator it=m_stEncode.mapKeys.begin(); it!=m_stEncode.mapKeys.end(); ++it )
            {
                assert(it->second <= 0xFF);
                sBuf += (char)it->second;	//keyvalue
                sBuf += static_cast<char>(it->first.size());
                sBuf.append(it->first.data(), it->first.size());			//keyname
            }
        }
        else if ( m_stEncode.mapKeys.size() <= 0xFFFF )
        {
            stEncodeHead.ucKeyTableType = 1;
            for ( MAPKEY::const_iterator it=m_stEncode.mapKeys.begin(); it!=m_stEncode.mapKeys.end(); ++it )
            {
                assert(it->second <= 0xFFFF);
                unsigned short wKeyValue = htons((unsigned short)it->second);
                sBuf.append((char*)&wKeyValue, sizeof(wKeyValue));
                sBuf += static_cast<char>(it->first.size());
                sBuf.append(it->first.data(), it->first.size());			//keyname
            }
        }
        else
        {
            assert(false);
            throw Error("m_stEncode.mapKeys error: too large.");
        }
        memcpy((char*)(sBuf.data()+dwEncodeHeadPos), &stEncodeHead, sizeof(stEncodeHead));

    }
private:

    static void encode_integer(std::string& sBuf, const ValueHolder& value)
    {
        if ( value.integer < 0xFF )
        {
            sBuf.push_back((char)DType::Integer1);
            sBuf.push_back((char)value.integer);
        }
        else if ( value.integer <= 0xFFFF )
        {
            sBuf.push_back((char)DType::Integer2);
            unsigned short wTmp = htons(static_cast<unsigned short>(value.integer));
            sBuf.append(reinterpret_cast<char*>(&wTmp),sizeof(wTmp));
        }
        else if ( value.integer <= 0xFFFFFFFF )
        {
            sBuf.push_back((char)DType::Integer4);
            uint32_t dwTmp = htonl(static_cast<uint32_t>(value.integer));
            sBuf.append(reinterpret_cast<char*>(&dwTmp),sizeof(dwTmp));
        }
        else
        {
            sBuf.push_back((char)DType::Integer8);
            uint64_t ui64Tmp = htonll(value.integer);
            sBuf.append(reinterpret_cast<char*>(&ui64Tmp),sizeof(ui64Tmp));
        }
    }

    static void encode_bool(std::string& sBuf, const ValueHolder& value)
    {
        sBuf.push_back((char)DType::Bool);
        sBuf.push_back((char)value.integer);
    }

    static void encode_float(std::string& sBuf, const ValueHolder& value)
    {

        sBuf.push_back((char)DType::Float);

        string sValue = toStr(value.flValue);
        sBuf += static_cast<char>(sValue.size());
        sBuf.append(sValue.data(),sValue.size());
    }


    static void encode_string(std::string& sBuf, const ValueHolder& value)
    {
        if ( value.buf->size() <= 0xFF )
        {
            sBuf.push_back((char)DType::String1);
            sBuf += static_cast<char>(value.buf->size());
        }
        else if ( value.buf->size() <= 0xFFFF )
        {
            sBuf.push_back((char)DType::String2);
            unsigned short wSize = htons(static_cast<unsigned short>(value.buf->size()));
            sBuf.append(reinterpret_cast<char*>(&wSize), sizeof(wSize));
        }
        else
        {
            sBuf.push_back((char)DType::String4);
            uint32_t dwSize = htonl(static_cast<uint32_t>(value.buf->size()));
            sBuf.append(reinterpret_cast<char*>(&dwSize), sizeof(dwSize));
        }
        sBuf.append(value.buf->data(), value.buf->size());
    }

    static void encode_vector(std::string& sBuf, const ValueHolder& value)
    {
        sBuf.push_back((char)DType::Vector);
        uint32_t dwSize = htonl(static_cast<uint32_t>(value.vec->size()));
        sBuf.append(reinterpret_cast<char*>(&dwSize), sizeof(dwSize));
        for(VecType::iterator it=value.vec->begin(); it!=value.vec->end(); ++it)
        {
            it->encodeNM(sBuf);
        }
    }

    static void encode_map(std::string& sBuf, const ValueHolder& value)
    {
        sBuf.push_back((char)DType::Map);
        size_t dwSizePos = sBuf.size();
        uint32_t dwSize = 0;//htonl(static_cast<unsigned long>(value.map->size()));
        sBuf.append(reinterpret_cast<char*>(&dwSize), sizeof(dwSize));
        for(MapType::iterator it=value.map->begin(); it!=value.map->end(); ++it)
        {
            ++dwSize;
            sBuf.push_back((char)it->first.size());
            sBuf.append(it->first.data(), it->first.size());
            it->second.encodeNM(sBuf);
        }
        dwSize = htonl(dwSize);
        memcpy((char*)sBuf.data()+dwSizePos, reinterpret_cast<char*>(&dwSize), sizeof(dwSize));
    }


    static void encode_none(std::string& sBuf, const ValueHolder& value)
    {

        sBuf.push_back((char)DType::Null);
    }

public:

    void encodeNM(std::string& sBuf)
    {

        switch(m_ucType)
        {

        case DType::Bool:
            encode_bool(sBuf, m_value);
            break;
        case DType::Integer:
            encode_integer(sBuf, m_value);
            break;
        case DType::Float:
            encode_float(sBuf, m_value);
            break;
        case DType::String:
            encode_string(sBuf, m_value);
            break;
        case DType::Vector:
            encode_vector(sBuf, m_value);
            break;
        case DType::Map:
            encode_map(sBuf, m_value);
            break;
        case DType::Null:
            encode_none(sBuf, m_value);
            break;
        default:
            encode_none(sBuf, m_value);
            break;
        }
    }

private:

    bool isHasData() const
    {
        return m_bHasData;
    }
    void init()
    {
        if ( !m_bInit )
        {
            memset(&m_value, 0, sizeof(m_value));
            m_bInit = true;
        }
    }
    void InitAsMap()
    {
        init();
        assert(DType::Null==m_ucType || DType::Map==m_ucType);
        if ( DType::Null==m_ucType || DType::Map==m_ucType )
        {
            if ( NULL == m_value.map )
            {
                m_ucType = DType::Map;
                m_ucSubType = DType::Map;
                m_value.map = new MapType;
            }
        }
        else
        {
            throw Error("InitAsMap error:type error.");
        }
    }
    void InitAsVector()
    {
        init();
        assert(DType::Null==m_ucType || DType::Vector==m_ucType);
        if ( DType::Null==m_ucType || DType::Vector==m_ucType )
        {
            if ( NULL == m_value.vec )
            {
                m_ucType = DType::Vector;
                m_ucSubType = DType::Vector;
                m_value.vec = new VecType;
            }
        }
        else
        {
            throw Error("InitAsVector error:type error.");
        }
    }
    void InitAsBuf()
    {
        init();
        assert(DType::Null==m_ucType || DType::String==m_ucType);
        if ( DType::Null==m_ucType || DType::String==m_ucType )
        {
            if ( NULL == m_value.buf )
            {
                m_ucType = DType::String;
                m_ucSubType = DType::String;
                m_value.buf = new BufType;
            }
        }
        else
        {
            throw Error("InitAsBuf error:type error.");
        }
    }

    void assign(const this_type& rhs)
    {

        if ( m_ucType == DType::String ) delete m_value.buf;
        if ( m_ucType == DType::Vector ) delete m_value.vec;
        if ( m_ucType == DType::Map ) delete m_value.map;

        if ( rhs.m_ucType == DType::Integer )
        {
            m_value.integer = rhs.m_value.integer;
        }

        if ( rhs.m_ucType == DType::Bool )
        {
            m_value.integer = rhs.m_value.integer;
        }

        if ( rhs.m_ucType == DType::Float )
        {
            m_value.flValue = rhs.m_value.flValue;
        }

        else if ( rhs.m_ucType == DType::Map )
        {
            assert(rhs.m_value.map != NULL);
            m_value.map = new MapType(*(rhs.m_value.map));
        }
        else if ( DType::Vector ==  rhs.m_ucType )
        {
            assert(rhs.m_value.vec != NULL);
            m_value.vec = new VecType(*rhs.m_value.vec);
        }
        else if ( DType::String ==  rhs.m_ucType )
        {
            assert(rhs.m_value.buf != NULL);
            m_value.buf = new BufType((*rhs.m_value.buf));
        }
        m_ucType = rhs.m_ucType;
        m_ucSubType = rhs.m_ucSubType;
        m_bHasData = rhs.m_bHasData;
        m_bInit = rhs.m_bInit;
        m_sEncodeData = rhs.m_sEncodeData;
    }



    void writeKeyType(std::string& sBuf, SEncode& stEncode, const unsigned char ucValueType)
    {
        if ( stEncode.bNeedTransform )
        {
            int iKeyType = stEncode.iKeyValue*13 + ucValueType;
            if ( stEncode.ucUsingByteCount==1 && iKeyType > 255 )
            {
                stEncode.ucUsingByteCount = 2;
                //write ext type tell to encode using 2 byte.
            }

            if ( stEncode.ucUsingByteCount == 1 )
            {
                sBuf.push_back((char)iKeyType);
            }
            else if ( stEncode.ucUsingByteCount == 2 )
            {
                unsigned short wTmp = htons(static_cast<unsigned short>(iKeyType));
                sBuf.append(reinterpret_cast<char*>(&wTmp),sizeof(wTmp));
            }
            else
            {
                assert(false);
                throw Error("stEncode.ucUsingByteCount error.");
            }

            stEncode.bNeedTransform = false;
        }
        else
        {
            sBuf.push_back((char)ucValueType);
        }
    }

private:
    unsigned char m_ucType;
    unsigned char m_ucSubType;
    ValueHolder m_value;
    bool m_bInit;
    bool m_bHasData;

    std::string m_sEncodeData;
    SEncode m_stEncode;
    SDecode m_stDecode;


};



template<typename T>

class CAnyValuePackage
{
public:
    typedef std::runtime_error Error;
    typedef CAnyValuePackage this_type;
    typedef T PKG_HEAD;

public:
    CAnyValuePackage()
    {
        m_sEncodeData.assign(sizeof(PKG_HEAD), 0);
    }

    PKG_HEAD& head()
    {
        return *(PKG_HEAD*)m_sEncodeData.data();
    }

    void sethead(const PKG_HEAD& stHead)
    {
        memcpy((char*)m_sEncodeData.data(), &stHead, sizeof(stHead));
    }

    ~CAnyValuePackage() {}

    void encodeJSON()
    {
        m_sEncodeData.erase(sizeof(PKG_HEAD));
        m_oAnyValues.encodeJSON(m_sEncodeData);
    }

    void encodeXML(const bool bUtf8 = true)
    {
        m_sEncodeData.erase(sizeof(PKG_HEAD));

        if (bUtf8)
            m_sEncodeData += "<?xml version=\"1.0\" encoding=\"UTF-8\"?><xmldata>";
        else
            m_sEncodeData += "<?xml version=\"1.0\" encoding=\"GB2312\"?><xmldata>";

        m_oAnyValues.encodeXML(m_sEncodeData);
        m_sEncodeData += "</xmldata>";
    }

    void encode(const unsigned char ucEncodeType=EncodeType::NORMAL)
    {
        //清除包体数据
        m_sEncodeData.erase(sizeof(PKG_HEAD));

        m_sEncodeData.append((char*)&ucEncodeType, sizeof(ucEncodeType));

        if ( ucEncodeType == EncodeType::TYPE1 )
        {
            m_oAnyValues.encodeOpt(m_sEncodeData);
        }
        else
        {
            m_oAnyValues.encodeNM(m_sEncodeData);
        }
    }

    void decode(const unsigned char* pData, const size_t dwDataSize)
    {
        this->decode((char*)pData, dwDataSize);
    }


    void decode(const char* pData, const size_t dwDataSize)
    {

        if ( dwDataSize < sizeof(PKG_HEAD) )
        {
            assert(false);
            throw Error("decode error:dwDataSize < sizeof(PKG_HEAD)");
        }
        m_oAnyValues.clear();
        m_sEncodeData.assign(pData, sizeof(PKG_HEAD));

        if ( dwDataSize < sizeof(PKG_HEAD)+1 )
        {
            return ;
        }
        m_oAnyValues.decode(sizeof(PKG_HEAD),pData,dwDataSize);
    }

    const char* data()
    {
        return m_sEncodeData.data();
    }
    size_t size()
    {
        return m_sEncodeData.size();
    }

    const char* bodydata()
    {
        return m_sEncodeData.data()+sizeof(PKG_HEAD);
    }
    const size_t bodysize()
    {
        return m_sEncodeData.size()-sizeof(PKG_HEAD);
    }

    void setbodydata(const unsigned char* pData, const size_t dwSize)
    {
        this->setbodydata((char*)pData, dwSize);
    }
    void setbodydata(const char* pData, const size_t dwSize)
    {
        m_sEncodeData.replace(sizeof(PKG_HEAD),std::string::npos,  pData,dwSize);
    }

    const CAnyValue& operator[](const std::string& sName) const
    {
        return m_oAnyValues[sName];
    }

    CAnyValue& operator[](const std::string& sName)
    {
        return m_oAnyValues[sName];
    }

    bool hasKey(const std::string& sName) const
    {
        return m_oAnyValues.hasKey(sName);
    }
    void insert(const std::string& sName, const CAnyValue& oValue)
    {
        m_oAnyValues.insert(sName, oValue);
    }

    void erase(const std::string& sName)
    {
        m_oAnyValues.erase(sName);
    }

    void clear()
    {
        m_oAnyValues.clear();
        m_sEncodeData.erase(sizeof(PKG_HEAD));
    }
    void clearbody()
    {
        m_oAnyValues.clear();
    }

    const CAnyValue& root() const
    {
        return m_oAnyValues;
    }


    void setroot(const CAnyValue& any)
    {
        m_oAnyValues = any;
    }

private:
    void assign(const this_type& rhs)
    {
        m_sEncodeData = rhs.m_sEncodeData;
    }
private:
    std::string m_sEncodeData;
    CAnyValue m_oAnyValues;

};

};



#endif

