#include <iostream>
#include <CAsyncLog.h>

using namespace std;
using namespace lce;

int main(int argc,char **argv)
{
 
	CAsyncLog logAsync;

	logAsync.init("./test",1,1024*1024*10);

	for(int i=0;i<10;i++)
	{
		for(int j=0;j<100;j++)
			logAsync.write("%d,%s",1,"abdfddsaaaaaaaaaaaadsasdadsasad1111111111111111222222222222222222222222222222222222222222222222222222222222222222222222111111111111111111111111111111111111111111111111111111");

		usleep(500000);
	}

	sleep(5);

	return 0;
}
