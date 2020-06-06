#include "../threadpool.h"
#include <pthread.h>
#include <iostream>
#include <functional>
#include <string>
#include <unistd.h>
using namespace std;
using namespace tudou;


int main()
{
	ThreadPool pool(10);
	//function<void()> ret = 
	for(int i=0; i<20; ++i)
	{
		if(i%2)
		bool flag = pool.addTask(
			[&](){ cout << "haha" << endl;}
			//bind(teak)
		);
		
		//pool.addTask(bind(tt));
	}	
}

