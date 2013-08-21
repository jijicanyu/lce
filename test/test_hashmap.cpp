#include "../CHashMap.h"
#include "../CShmHashMap.h"
#include <iostream>
using namespace  std;
using namespace lce;
int main(int argc, char **argv)
{
	CHashMap<int> hashMap;
	hashMap.init(1);
	cout<<hashMap.empty()<<endl;
	hashMap.insert(123,1231111);

	CHashMap<int>::iterator it = hashMap.find(123);

	cout<<hashMap.empty()<<endl;

	cout<<hashMap.full()<<endl;

	cout<<*it<<endl;
	hashMap.erase(it);
	cout<<hashMap.empty()<<endl;

	cout<<hashMap.full()<<endl;
	return 0;
}