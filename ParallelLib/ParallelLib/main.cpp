#include <iostream>
#include <sstream>

#include "ParallelLib.h"

int main()
{
#if 0
	std::cout << "parallel test 1: four threads, join\n";

	create_public(int, s, 10);
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
				std::stringstream out;
				out << "A: (" << p << ") " << s << "\n";
				printf("%s", out.str().c_str());
			});

		parallel_do(parTest2b, num_threads(2) nowait(yes) exec_master(no),
			{
				for (int i = 0; i < INT_MAX; ++i);
				p = THREAD_ID;
				++s;
				std::stringstream out;
				out << "B: (" << p << ") " << s << "\n";
				printf("%s", out.str().c_str());
			});
	}

	std::cout << "end of nojoin scope - joined\n";

	std::cout << "\n\nparallelFor test 1:\n  for(i = 0; i < 40; i += 4) in six threads (static; chunk:1)\n";

	pFor forTest;

	forTest.NoWait(false)
		.ExecuteOnMaster(false)
		.Schedule(pSchedule::Static)
		.ChunkSize(1)
		.NumThreads(8)
		.Do(0, 40, 4, [&](pExecParams ___pExecParams, int iterator)
	{
		std::stringstream result;
		result << iterator << "\t@ " << THREAD_ID << "\n";
		printf("%s", result.str().c_str());
	});

	std::cout << "\n\nparallelFor test 2:\n  for(i = 0; i < 44; i += 4) in five threads (dynamic; chunk:2)\n";

	pFor forTest2;

	forTest2.NoWait(false)
		.ExecuteOnMaster(false)
		.Schedule(pSchedule::Dynamic)
		.ChunkSize(1)
		.NumThreads(7)
		.Do(0, 44, 4, [&](pExecParams ___pExecParams, int iterator)
	{
		___pExecParams.SleepMili((THREAD_ID - 8) * 200.0f);
		std::stringstream result;
		result << iterator << "\t@ " << THREAD_ID << "\n";
		printf("%s", result.str().c_str());
	});

	std::cout << "\n\nparallelFor test 3:\n  for(i = 87; i > 3; i -= 7) in three threads (static; chunk:3)\n";

	pFor forTest3;

	forTest3.NoWait(false)
		.ExecuteOnMaster(false)
		.Schedule(pSchedule::Static)
		.NumThreads(3)
		.Do(87, 3, -7, [&](pExecParams ___pExecParams, int iterator)
	{
		___pExecParams.SleepMili((THREAD_ID - 8) * 200.0f);
		std::stringstream result;
		result << iterator << "\t@ " << THREAD_ID << "\n";
		printf("%s", result.str().c_str());
	});

	std::cout << "\n\nparallelFor test 4:\n  for(i = 87; i > 3; i -= 7) in three threads (dynamic; chunk:3)\n";

	pFor forTest4;

	forTest4.NoWait(false)
		.ExecuteOnMaster(false)
		.Schedule(pSchedule::Dynamic)
		.ChunkSize(3)
		.NumThreads(3)
		.Do(87, 3, -7, [&](pExecParams ___pExecParams, int iterator)
	{
		___pExecParams.SleepMili((THREAD_ID - 8) * 200.0f);
		std::stringstream result;
		result << iterator << "\t@ " << THREAD_ID << "\n";
		printf("%s", result.str().c_str());
	});
#endif

	std::cout << "\n\nparallelSecetions test 1:\n  five sections C++ style (-nowait, +exec_master)\n";
	pSections sections; sections
		.NoWait(false)
		.ExecuteOnMaster(true)
		.Do({
				[&](pExecParams ___pExecParams)
				{
					PARALLEL_SLEEP_MILISECONDS(THREAD_ID);
					printf("section A (thread %d)\n", THREAD_ID);
				},
				[&](pExecParams ___pExecParams)
				{
					PARALLEL_SLEEP_MILISECONDS(THREAD_ID);
					printf("section B (thread %d)\n", THREAD_ID);
				},
				[&](pExecParams ___pExecParams)
				{
					PARALLEL_SLEEP_MILISECONDS(THREAD_ID);
					printf("section C (thread %d)\n", THREAD_ID);
				},
				[&](pExecParams ___pExecParams)
				{
					PARALLEL_SLEEP_MILISECONDS(THREAD_ID);
					printf("section D (thread %d)\n", THREAD_ID);
				},
				[&](pExecParams ___pExecParams)
				{
					PARALLEL_SLEEP_MILISECONDS(THREAD_ID);
					printf("section E (thread %d)\n", THREAD_ID);
				},
			});

	std::cout << "\n\nparallelSecetions test 2:\n  three sections OMP style (+nowait, -exec_master)\n";
	parallel_sections(sections2, nowait(yes) exec_master(no),
		parallel_section
		{
			PARALLEL_SLEEP_MILISECONDS(THREAD_ID);
			printf("defined section A (thread %d)\n", THREAD_ID);
		}
		parallel_section
		{
			PARALLEL_SLEEP_MILISECONDS(THREAD_ID);
			printf("defined section B (thread %d)\n", THREAD_ID);
		}
		parallel_section
		{
			PARALLEL_SLEEP_MILISECONDS(THREAD_ID);
			printf("defined section C (thread %d)\n", THREAD_ID);
		}
		)

	std::cout << "--done--";

	std::getchar();
	return 0;
}
