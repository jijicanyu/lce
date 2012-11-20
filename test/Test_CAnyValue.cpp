#include <iostream>
#include "../CAnyValue.h"
#include <stdlib.h>
using namespace std;
using namespace lce;

int main(int argc,char *argv[])
{

    CAnyValue oValue;
    oValue["abc"]=1;
    oValue["bcd"]="hello";
    oValue["aa"].push_back("aaa");
    oValue["aa"].push_back(0);
    oValue["aa"].push_back(CAnyValue::m_null);


    oValue["xbv"]["xx"]=1.5;
    oValue["xbv"]["ms"]="xxxx";
    oValue["xbv"]["xxxx"] =true;
    oValue["xbv"]["x"];

	string sData;
	string sData2;
    oValue.encodeJSON(sData);
	oValue.decodeJSON(sData.data(),sData.size());
    oValue.encodeJSON(sData);
	oValue.encode(sData2);
	oValue.clear();
	size_t iPos = 0;
	oValue.decode(iPos,sData2.data(),sData2.size());
	oValue.encodeJSON(sData);
    cout<< sData<<endl;

    return 0;
}
