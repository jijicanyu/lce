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
#include <list>
#include <algorithm>
#include <string>
#include <fcntl.h>
#include "../CShmList.h"
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

	CShmList<int> liData;
	liData.init(0x1111,4096,true);
	cout<<"maxsize ="<<liData.maxSize()<<endl;


	for(int i=0;i<80;i++)
	{
		if (!liData.push_back(i))
		{
			cout<<liData.getErrMsg()<<endl;
		}
		cout<<"size="<<liData.size()<<endl;
	}

	int sizet = liData.size();
	for(int j=0;j<sizet;j++)
	{
		cout<<liData.front()<<endl;
		liData.pop_front();
	}

    return 0;
}

