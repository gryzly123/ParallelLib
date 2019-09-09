#include <iostream>
#include <atomic>
#include "ParallelDo.h"
#include "ParallelFor.h"

#define yes true
#define no false
#define parallel_do(tag, params, method) pDo tag; tag params .Do([&](pExecParams ___pExecParams) method);
#define parallel_for(tag, iterator, init_val, max_val, increment, params, method) pFor tag; tag params .Do(init_val, max_val, increment, [](int iterator) method);
#define num_threads(n)                    .NumThreads(n)
#define exec_master(c)                  .ExecuteOnMaster(c)
#define nowait(c)                       .NoWait(c)

#define create_public(type, name, value) std::atomic<type> name
#define create_private(type, name) thread_local type name


int main()
{
	std::cout << "parallel test 1: four threads, join\n";

#if 0
	create_public(int, s) = 10;
	create_private(int, p);

	parallel_do(parTest1, num_threads(4) nowait(no) exec_master(yes),
	{
		for (int i = 0; i < INT_MAX; ++i);
		p = THREAD_ID;
		++s;
		std::cout << p << " " << s << "\n";
	});

	std::cout << "parallel test 2: four threads, nojoin\n";

	{
		parallel_do(parTest2a, num_threads(2) nowait(yes) exec_master(no),
		{
			for (int i = 0; i < INT_MAX; ++i);
			p = THREAD_ID;
			++s;
			std::cout << p << " " << s << "\n";
		});
		
		parallel_do(parTest2b, num_threads(2) nowait(yes) exec_master(no), 
		{
			for (int i = 0; i < INT_MAX; ++i);
			p = THREAD_ID;
			++s;
			std::cout << p << " " << s << "\n";
		});
	}

	std::cout << "end of nojoin scope - joined\n";
#endif

	//parallel_for(forTest1, iter, 0, 100, 4, num_threads(2) nowait(yes) exec_master(no),
	//{
	//	std::cout << iter << "\n";
	//});

	//pFor forTest;
	//
	//const int init_val = 0;
	//const int max_val = 100;
	//const int increment = 4;
	//
	//forTest.Do(init_val, max_val, increment, [&](int iterator) 
	//	{
	//		std::cout << iterator << "\n";
	//	});

	std::getchar();
	return 0;
}
