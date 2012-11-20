#include <iostream>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <queue>
#include <set>
#include <algorithm>
#include <string>
#include <fcntl.h>
#include "../CShmHashMap.h"
using namespace std;
using namespace lce;

#pragma pack(1)

struct SData
{
    int ab;
    char szBuf[20];
    char a;
};

#pragma pack()

int main(int argc,char *argv[])
{


	SData stData;
	stData.ab=123;
	stData.a='X';

	CShmHashMap<SData> hashmapData;

	hashmapData.init(0x1000,1024*1024);

	/*
	hashmapData.insert(1,stData);


	stData.a='m';

	hashmapData.insert(2,stData);
	*/
	cout<<hashmapData.find(1)->second.a<<endl;
	cout<<hashmapData.find(2)->second.a<<endl;
    return 0;
}

