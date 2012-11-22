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
#include <fcntl.h>
#include "../CShm.h"
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

	CShm oShm;
	oShm.create(0x1111,1024*10);

	cout<<(long)oShm.getShmBuf()<<endl;
    return 0;
}

