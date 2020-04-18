#pragma once
#include <random>
#include <omp.h>

//https://dsp.krzaq.cc/post/180/nie-uzywaj-rand-cxx-ma-random/
thread_local std::mt19937 gen2{ std::random_device{}() };

int losuj(int min, int max) {
	return std::uniform_int_distribution<int>{min, max}(gen2);
}

#include <dlib/threads.h>   //for dlib::mutex, signaler, thread
#include <dlib/misc_api.h>  //for dlib::sleep
#include "dlib/threads/parallel_for_extension.h"
#include <functional>

//zmienne zarz¹dzaj¹ce regionem oraz wspó³dzielone
const int numThreads = 3;
int threadCount = numThreads;
dlib::mutex count_mutex;
dlib::signaler count_signaler(count_mutex);
int wspoldzielona = 0;
dlib::mutex critical;

//definicja cia³a fragmentu zrównoleglonego
void parallel_do_body(void*)
{
	int prywatna = losuj(0, 5);
	{
		dlib::auto_mutex scope_guard(critical);
		wspoldzielona += prywatna;
		printf("Watek %d ma private=%d, nowa shared=%d\n",
			dlib::get_thread_id(),
			prywatna,
			wspoldzielona);
	}

	dlib::auto_mutex scope_guard(critical);
	--threadCount;
	count_signaler.signal();
}

int main2()
{
	//utworzenie w¹tków
	for (int i = numThreads; i > 0; --i)
		dlib::create_new_thread(parallel_do_body, 0);

	//oczekiwanie na zakoñczenie pracy w¹tków
	while (threadCount > 0)
		count_signaler.wait();

	/////////////////////////////////////////

	int tablica[100];
	dlib::thread_pool tpool(4);

	dlib::parallel_for(tpool, 0, 100, [&](long i)
	{
		tablica[i] = losuj(0, i);
	});

	for (int i = 0; i < 100; ++i) printf("%d\n", tablica[i]);

	/////////////////////////////////////////

	return 0;
}


/*
#include "tbb/iterators.h"
#include "tbb/mutex.h"
#include "tbb/parallel_do.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"
#include "tbb/parallel_invoke.h"
#include "tbb/task_scheduler_init.h"
#include <chrono>
void looop()
{
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	tbb::mutex critical;
	tbb::parallel_for(0, 10, [&](size_t i)
	{
		Sleep(1000);
	});

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	unsigned long long a = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << a << "seconds\n";
}

void loop() //10 iteracji po 1s
{
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	tbb::mutex critical;

	tbb::parallel_do(tbb::counting_iterator<int>(0), tbb::counting_iterator<int>(10),
		[](int thread_id, tbb::parallel_do_feeder<int>& feeder)
		{
			printf("%d ", thread_id);
			Sleep(1000);
		}
	);
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	unsigned long long a = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << a << "seconds\n";

//	tbb::parallel_for(0, 10, [&](size_t i) { Sleep(1000); });
};

int main2()
{

{
	tbb::task_scheduler_init tbbScheduler(2);
	loop(); //5s (2w5s)
	{
		tbb::task_scheduler_init tbbScheduler2(3);
		loop(); //5s (2w5s) - drugi obiekt jest ignorowany
	}
	loop(); //5s (2w5s) - pierwszy obiekt nadal istnieje
}
{
	tbb::task_scheduler_init tbbScheduler2(3);
	loop(); //4s (3w3s + 1w1s)
}
loop(); //3s (4w2s + 2w1s) - domyœlnie 4 w¹tki, brak obiektów schedulera
{
	tbb::task_scheduler_init tbbScheduler2(10);
	loop(); //3s (4w2s + 2w1s) - max. 10 w¹tków, ale TBB uzna³, ¿e op³aca siê u¿yæ 4
}


/////////////////////////////////////////

int wspoldzielona = 0;
tbb::mutex critical;

tbb::parallel_do(tbb::counting_iterator<int>(0), tbb::counting_iterator<int>(3),
	[&wspoldzielona, &critical](int thread_id, tbb::parallel_do_feeder<int>& feeder)
	{
	int prywatna = losuj(0, 5);
	tbb::mutex::scoped_lock scope_guard(critical);
	wspoldzielona += prywatna;
	printf("Watek %d ma private=%d, nowa shared=%d\n",
		thread_id,
		prywatna,
		wspoldzielona);
	}
);

	/////////////////////////////////////////

	int tablica[100];

tbb::parallel_for(0, 100, 
	[&](size_t i)
	{
		tablica[i] = losuj(0, i);
	},
	tbb::static_partitioner());

	tbb::parallel_for(0, 100,
		[&](size_t i)
		{
			tablica[i] = losuj(0, i);
		});

	int tablica2[30][20];

	tbb::parallel_for(tbb::blocked_range2d<size_t, size_t>(0, 20, 0, 30),
		[&tablica2](const tbb::blocked_range2d<size_t, size_t> &range)
		{
			const size_t i_end = range.rows().end();
			const size_t j_end = range.cols().end();

			for (size_t i = range.rows().begin(); i < i_end; ++i)
				for (size_t j = range.cols().begin(); j < j_end; ++j)
					tablica2[j][i] = losuj(i, j);
		});

	/////////////////////////////////////////

tbb::parallel_invoke(
	[&]()
	{
		int a = 1;
		printf("Pierwsza sekcja\n");
	},
	[&]()
	{
		int a = 2;
		printf("Druga sekcja\n");
	});

	return 0;
}
*/
/*
#include "ParallelLib/ParallelLib.h"
#include <functional>
int main2()
{
	/////////////////////////////////////////

// --- MAKRA ---
int wspoldzielona = 0;
std::mutex critical;

parallel_do(region, num_threads(3), 
{
	int prywatna = losuj(0, 5);

	const std::lock_guard<std::mutex> scope_guard(critical);
	wspoldzielona += prywatna;
	printf("Watek %d ma private=%d, nowa shared=%d\n",
		THREAD_ID,
		prywatna,
		wspoldzielona);
});
// --- C++11 ---
int wspoldzielona = 0;
thread_local int prywatna;

pDo region2; region2
.NumThreads(3)
.Do([&](pExecParams params)
{
	prywatna = losuj(0, 5);
	wspoldzielona += prywatna;
	printf("Watek %d ma private=%d, nowa shared=%d\n",
		params.ThreadId,
		prywatna,
		wspoldzielona);
});

	/////////////////////////////////////////


// --- MAKRA ---
int tablica[100];

parallel_for(petla, i, 0, 100, 1, schedule(Static, 1),
{
	tablica[i] = losuj(0, i);
});

// --- C++11 ---
int tablica[100];

pFor petla2; petla2
.Schedule(pSchedule::Static)
.Do(0, 100, 1, [&](pExecParams ___pExecParams, int i)
{
	tablica[i] = losuj(0, i);
});


	/////////////////////////////////////////


// --- MAKRA ---
parallel_sections(sekcje, ,
	parallel_section
	{
		int a = 1;
		printf("Pierwsza sekcja\n");
	}
	parallel_section
	{
		int a = 2;
		printf("Druga sekcja\n");
	}
);

// --- C++11 ---
pSections sekcje2; sekcje2.Do(
{
	[&](pExecParams ___pExecParams)
	{
		int a = 1;
		printf("Pierwsza sekcja\n");
	},
	[&](pExecParams ___pExecParams)
	{
		int a = 2;
		printf("Druga sekcja\n");
	}
});
	printf("%d\n", a);

	return 0;
}
*/

//OPENMP
/*
int main2()
{
int wspoldzielona = 0;
int prywatna;

#pragma omp parallel shared(wspoldzielona) private(prywatna) num_threads(3)
{
	prywatna = losuj(0, 5);
	#pragma omp critical
	{
		wspoldzielona += prywatna;
		printf("Watek %d ma private=%d, nowa shared=%d\n",
			omp_get_thread_num(),
			prywatna,
			wspoldzielona);
	}
}

int tablica[100];

#pragma omp parallel for schedule(static, 5)
for (int i = 0; i < 100; ++i)
{
	tablica[i] = losuj(0, i);
}

int a = 0;
#pragma omp parallel sections lastprivate(a)
{
#pragma omp section
	{
		a = 1;
		printf("Pierwsza sekcja\n");
	}
#pragma omp section
	{
		a = 2;
		printf("Druga sekcja\n");
	}
}
printf("%d\n", a);

return 0;
}
*/
