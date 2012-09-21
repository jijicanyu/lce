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
    oValue.encodeJSON();
    cout<< oValue.data()<<endl;

    return 0;
}
