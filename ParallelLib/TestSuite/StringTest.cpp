#include "stdafx.h"
#include "StringTest.h"

//-------------------- rand engine

//https://dsp.krzaq.cc/post/180/nie-uzywaj-rand-cxx-ma-random/
thread_local std::mt19937 gen{ std::random_device{}() };

int random(int min, int max) {
	return std::uniform_int_distribution<int>{min, max}(gen);
}

//-------------------- construction

StringTest::StringTest(const std::string& inName, const StringTestConfig& config)
	: Test(TestType::StringTest, inName)
	, testConfig(config)
{

}

StringTest::~StringTest() { }

//-------------------- algoritm

#define in_range(x,a,b) ((x) >= (a) && (x) <= (b))

void StringTest::GenerateRandomString(char* buffer, int len)
{
	char* buffer_max = buffer + len;
	bool is_capital = false;
	do
	{
		*buffer = is_capital ? random('A', 'Z') : random('a', 'z');
		is_capital = !is_capital;
	} 
	while (++buffer < buffer_max);
	*(buffer - 1) = '\0'; //don't forget null terminator
}

void StringTest::StringToUpper(char* buffer, int len)
{
	char* buffer_max = buffer + len;
	do
	{
		*buffer -= in_range(*buffer, 'a', 'z') ? 32 : 0; /* 'A' - 'a' = -32 */
	} while (++buffer < buffer_max);
}

void StringTest::StringToLower(char* buffer, int len)
{
	char* buffer_max = buffer + len;
	do
	{
		*buffer += in_range(*buffer, 'A', 'Z') ? 32 : 0; /* 'a' - 'A' = +32 */
	} while (++buffer < buffer_max);
}

void StringTest::StringAlternate(char* buffer, int len)
{
	char* buffer_max = buffer + len;
	do
	{
		if (in_range(*buffer, 'A', 'Z')) *buffer += 32;
		else if(in_range(*buffer, 'a', 'z')) *buffer -= 32;
	} while (++buffer < buffer_max);
}

//-------------------- implementations

void StringTest::DoSequentially(const TestParams& In, RetryResult& Out)
{
	Out.BeginResourceInit();
	{
		StringList ProducedStrings, UpperStrings, LowerStrings, AlternatedStrings;
		StringList* ProducedPtr   = nullptr;
		StringList* UpperPtr      = nullptr;
		StringList* LowerPtr      = nullptr;
		StringList* AlternatedPtr = nullptr;

		Out.BeginParallelWorkload();

		for (int i = 0; i < testConfig.numStringsToGenerate; ++i)
		{
			StringList* NewProduced = (ProducedPtr != nullptr) ? new StringList() : &ProducedStrings;
			NewProduced->allocate_string(testConfig.stringLen);
			GenerateRandomString(NewProduced->string, NewProduced->len);
			if (ProducedPtr != nullptr)
			{
				ProducedPtr->next = NewProduced;
				ProducedPtr = ProducedPtr->next;
			}
			else
			{
				ProducedPtr = NewProduced;
			}
		}

		StringList* LastProcessedProduced = &ProducedStrings;
		do
		{
			StringList* NewUpper = (UpperPtr != nullptr) ? new StringList() : &UpperStrings;
			NewUpper->allocate_string(testConfig.stringLen);
			memcpy(NewUpper->string, LastProcessedProduced->string, LastProcessedProduced->len);
			StringToUpper(NewUpper->string, NewUpper->len);
			if (UpperPtr != nullptr)
			{
				UpperPtr->next = NewUpper;
				UpperPtr = UpperPtr->next;
			}
			else
			{
				UpperPtr = NewUpper;
			}
		}
		while ((LastProcessedProduced = LastProcessedProduced->next) != nullptr);

		LastProcessedProduced = &ProducedStrings;
		do
		{
			StringList* NewLower = (LowerPtr != nullptr) ? new StringList() : &LowerStrings;
			NewLower->allocate_string(testConfig.stringLen);
			memcpy(NewLower->string, LastProcessedProduced->string, LastProcessedProduced->len);

			StringToLower(NewLower->string, NewLower->len);
			if (LowerPtr != nullptr)
			{
				LowerPtr->next = NewLower;
				LowerPtr = LowerPtr->next;
			}
			else
			{
				LowerPtr = NewLower;
			}

		}
		while ((LastProcessedProduced = LastProcessedProduced->next) != nullptr);

		LastProcessedProduced = &ProducedStrings;
		do
		{
			StringList* NewAlternated = (AlternatedPtr != nullptr) ? new StringList() : &AlternatedStrings;
			NewAlternated->allocate_string(testConfig.stringLen);
			memcpy(NewAlternated->string, LastProcessedProduced->string, LastProcessedProduced->len);

			StringAlternate(NewAlternated->string, NewAlternated->len);
			if (AlternatedPtr != nullptr)
			{
				AlternatedPtr->next = NewAlternated;
				AlternatedPtr = AlternatedPtr->next;
			}
			else
			{
				AlternatedPtr = NewAlternated;
			}

		}
		while ((LastProcessedProduced = LastProcessedProduced->next) != nullptr);

		Out.BeginResourceCleanup();
		//printf("\n\n%s\n%s\n%s\n%s\n\n", ProducedStrings.string, UpperStrings.string, LowerStrings.string, AlternatedStrings.string);

		//note: data will be removed at the end of scope thanks to StringList's destructor
	}
	Out.EndTask(true);
}

#ifndef __GNUC__
#include <Windows.h>
#define ___sleep(ms) Sleep(ms)
#else
void ___sleep(int ms)
{
    struct timespec time, outtime;
    time.tv_sec = ms / 1000;
    time.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&time, &outtime);
}
#endif

//void ConsumerThread(StringList*& targetArray, bool& bProductionCompleted,)

void StringTest::DoOpenMP(const TestParams& In, RetryResult& Out)
{
	Out.BeginResourceInit();
	StringList* ProducedStrings = nullptr;
	StringList* UpperStrings = nullptr;
	StringList* LowerStrings = nullptr;
	StringList* AlternatedStrings = nullptr;

	bool bProductionCompleted = false;

	Out.BeginParallelWorkload();

	#pragma omp parallel sections
	{
		#pragma omp section
		{
			StringList* ProducedPtr = nullptr;
			for (int i = 0; i < testConfig.numStringsToGenerate; ++i)
			{
				StringList* NewProduced = new StringList();
				NewProduced->allocate_string(testConfig.stringLen);
				GenerateRandomString(NewProduced->string, NewProduced->len);

				#pragma omp critical(use_producer_object)
				{
					if (ProducedPtr != nullptr)
					{
						ProducedPtr->next = NewProduced;
						ProducedPtr = ProducedPtr->next;
					}
					else
					{
						ProducedStrings = NewProduced;
						ProducedPtr = NewProduced;
					}
				}
			}
			bProductionCompleted = true;
		}

		#pragma omp section
		{
			StringList* UpperPtr = nullptr;
			StringList* LastProcessedProduced = nullptr;
			bool bNothingToProcess = true;
			bool bStringsLeft = true;
			while(bStringsLeft)
			{
				while (bNothingToProcess && bStringsLeft)
				{
					#pragma omp critical(use_producer_object)
					{
						//we haven't consumed anything yet but there is something to consume
						if (LastProcessedProduced == nullptr && ProducedStrings != nullptr)
						{
							LastProcessedProduced = ProducedStrings;
							bNothingToProcess = false;
						}
						//is there next item to consume?
						else if (LastProcessedProduced != nullptr && LastProcessedProduced->next != nullptr)
						{
							LastProcessedProduced = LastProcessedProduced->next;
							bNothingToProcess = false;
						}
						//no more items and producer claims it's finished
						else if(bProductionCompleted)
						{
							bStringsLeft = false;
							bNothingToProcess = false;
						}
					}
					if (bNothingToProcess) ___sleep(1);
				}
				if (!bStringsLeft) break;

				StringList* NewUpper = new StringList();
				NewUpper->allocate_string(testConfig.stringLen);
				memcpy(NewUpper->string, LastProcessedProduced->string, LastProcessedProduced->len);
				StringToUpper(NewUpper->string, NewUpper->len);
				bNothingToProcess = true;
				if (UpperPtr != nullptr)
				{
					UpperPtr->next = NewUpper;
					UpperPtr = UpperPtr->next;
				}
				else
				{
					UpperStrings = NewUpper;
					UpperPtr = NewUpper;
				}
			}
		}

		#pragma omp section
		{
			StringList* LowerPtr = nullptr;
			StringList* LastProcessedProduced = nullptr;
			bool bNothingToProcess = true;
			bool bStringsLeft = true;
			while (bStringsLeft)
			{
				while (bNothingToProcess && bStringsLeft)
				{
					#pragma omp critical(use_producer_object)
					{
						//we haven't consumed anything yet but there is something to consume
						if (LastProcessedProduced == nullptr && ProducedStrings != nullptr)
						{
							LastProcessedProduced = ProducedStrings;
							bNothingToProcess = false;
						}
						//is there next item to consume?
						else if (LastProcessedProduced != nullptr && LastProcessedProduced->next != nullptr)
						{
							LastProcessedProduced = LastProcessedProduced->next;
							bNothingToProcess = false;
						}
						//no more items and producer claims it's finished
						else if (bProductionCompleted)
						{
							bStringsLeft = false;
							bNothingToProcess = false;
						}
					}
					if (bNothingToProcess) ___sleep(1);
				}
				if (!bStringsLeft) break;

				StringList* NewLower = new StringList();
				NewLower->allocate_string(testConfig.stringLen);
				memcpy(NewLower->string, LastProcessedProduced->string, LastProcessedProduced->len);

				StringToLower(NewLower->string, NewLower->len);
				bNothingToProcess = true;
				if (LowerPtr != nullptr)
				{
					LowerPtr->next = NewLower;
					LowerPtr = LowerPtr->next;
				}
				else
				{
					LowerStrings = NewLower;
					LowerPtr = NewLower;
				}
			}
		}

		#pragma omp section
		{
			StringList* AlternatedPtr = nullptr;
			StringList* LastProcessedProduced = nullptr;
			bool bNothingToProcess = true;
			bool bStringsLeft = true;
			while (bStringsLeft)
			{
				while (bNothingToProcess && bStringsLeft)
				{
					#pragma omp critical(use_producer_object)
					{
						//we haven't consumed anything yet but there is something to consume
						if (LastProcessedProduced == nullptr && ProducedStrings != nullptr)
						{
							LastProcessedProduced = ProducedStrings;
							bNothingToProcess = false;
						}
						//is there next item to consume?
						else if (LastProcessedProduced != nullptr && LastProcessedProduced->next != nullptr)
						{
							LastProcessedProduced = LastProcessedProduced->next;
							bNothingToProcess = false;
						}
						//no more items and producer claims it's finished
						else if (bProductionCompleted)
						{
							bStringsLeft = false;
							bNothingToProcess = false;
						}
					}
					if (bNothingToProcess) ___sleep(1);
				}
				if (!bStringsLeft) break;

				StringList* NewAlternated = new StringList();
				NewAlternated->allocate_string(testConfig.stringLen);
				memcpy(NewAlternated->string, LastProcessedProduced->string, LastProcessedProduced->len);

				StringAlternate(NewAlternated->string, NewAlternated->len);
				bNothingToProcess = true;
				if (AlternatedPtr != nullptr)
				{
					AlternatedPtr->next = NewAlternated;
					AlternatedPtr = AlternatedPtr->next;
				}
				else
				{
					AlternatedStrings = NewAlternated;
					AlternatedPtr = NewAlternated;
				}
			}
		}
	}

	Out.BeginResourceCleanup();
	
	delete ProducedStrings;
	delete UpperStrings;
	delete LowerStrings;
	delete AlternatedStrings;
	
	Out.EndTask(true);
}

#include "tbb/parallel_invoke.h"
#include "tbb/mutex.h"
void StringTest::DoTBB(const TestParams& In, RetryResult& Out)
{
	Out.BeginResourceInit();
	StringList* ProducedStrings = nullptr;
	StringList* UpperStrings = nullptr;
	StringList* LowerStrings = nullptr;
	StringList* AlternatedStrings = nullptr;
	tbb::mutex use_producer_object;

	bool bProductionCompleted = false;

	Out.BeginParallelWorkload();

	{
		tbb::parallel_invoke(
		[&]() {
			StringList* ProducedPtr = nullptr;
			for (int i = 0; i < testConfig.numStringsToGenerate; ++i)
			{
				StringList* NewProduced = new StringList();
				NewProduced->allocate_string(testConfig.stringLen);
				GenerateRandomString(NewProduced->string, NewProduced->len);

				{
					tbb::mutex::scoped_lock lock(use_producer_object);
					if (ProducedPtr != nullptr)
					{
						ProducedPtr->next = NewProduced;
						ProducedPtr = ProducedPtr->next;
					}
					else
					{
						ProducedStrings = NewProduced;
						ProducedPtr = NewProduced;
					}
				}
			}
			bProductionCompleted = true;
		},
		[&]() {
			StringList* UpperPtr = nullptr;
			StringList* LastProcessedProduced = nullptr;
			bool bNothingToProcess = true;
			bool bStringsLeft = true;
			while (bStringsLeft)
			{
				while (bNothingToProcess && bStringsLeft)
				{
					{
						tbb::mutex::scoped_lock lock(use_producer_object);
						//we haven't consumed anything yet but there is something to consume
						if (LastProcessedProduced == nullptr && ProducedStrings != nullptr)
						{
							LastProcessedProduced = ProducedStrings;
							bNothingToProcess = false;
						}
						//is there next item to consume?
						else if (LastProcessedProduced != nullptr && LastProcessedProduced->next != nullptr)
						{
							LastProcessedProduced = LastProcessedProduced->next;
							bNothingToProcess = false;
						}
						//no more items and producer claims it's finished
						else if (bProductionCompleted)
						{
							bStringsLeft = false;
							bNothingToProcess = false;
						}
					}
					if (bNothingToProcess) ___sleep(1);
				}
				if (!bStringsLeft) break;

				StringList* NewUpper = new StringList();
				NewUpper->allocate_string(testConfig.stringLen);
				memcpy(NewUpper->string, LastProcessedProduced->string, LastProcessedProduced->len);
				StringToUpper(NewUpper->string, NewUpper->len);
				bNothingToProcess = true;
				if (UpperPtr != nullptr)
				{
					UpperPtr->next = NewUpper;
					UpperPtr = UpperPtr->next;
				}
				else
				{
					UpperStrings = NewUpper;
					UpperPtr = NewUpper;
				}
			}
		},
			[&]() {
			StringList* LowerPtr = nullptr;
			StringList* LastProcessedProduced = nullptr;
			bool bNothingToProcess = true;
			bool bStringsLeft = true;
			while (bStringsLeft)
			{
				while (bNothingToProcess && bStringsLeft)
				{
					{
						tbb::mutex::scoped_lock lock(use_producer_object);
						//we haven't consumed anything yet but there is something to consume
						if (LastProcessedProduced == nullptr && ProducedStrings != nullptr)
						{
							LastProcessedProduced = ProducedStrings;
							bNothingToProcess = false;
						}
						//is there next item to consume?
						else if (LastProcessedProduced != nullptr && LastProcessedProduced->next != nullptr)
						{
							LastProcessedProduced = LastProcessedProduced->next;
							bNothingToProcess = false;
						}
						//no more items and producer claims it's finished
						else if (bProductionCompleted)
						{
							bStringsLeft = false;
							bNothingToProcess = false;
						}
					}
					if (bNothingToProcess) ___sleep(1);
				}
				if (!bStringsLeft) break;

				StringList* NewLower = new StringList();
				NewLower->allocate_string(testConfig.stringLen);
				memcpy(NewLower->string, LastProcessedProduced->string, LastProcessedProduced->len);

				StringToLower(NewLower->string, NewLower->len);
				bNothingToProcess = true;
				if (LowerPtr != nullptr)
				{
					LowerPtr->next = NewLower;
					LowerPtr = LowerPtr->next;
				}
				else
				{
					LowerStrings = NewLower;
					LowerPtr = NewLower;
				}
			}
		},
			[&]() {
			StringList* AlternatedPtr = nullptr;
			StringList* LastProcessedProduced = nullptr;
			bool bNothingToProcess = true;
			bool bStringsLeft = true;
			while (bStringsLeft)
			{
				while (bNothingToProcess && bStringsLeft)
				{
					{
						tbb::mutex::scoped_lock lock(use_producer_object);
						//we haven't consumed anything yet but there is something to consume
						if (LastProcessedProduced == nullptr && ProducedStrings != nullptr)
						{
							LastProcessedProduced = ProducedStrings;
							bNothingToProcess = false;
						}
						//is there next item to consume?
						else if (LastProcessedProduced != nullptr && LastProcessedProduced->next != nullptr)
						{
							LastProcessedProduced = LastProcessedProduced->next;
							bNothingToProcess = false;
						}
						//no more items and producer claims it's finished
						else if (bProductionCompleted)
						{
							bStringsLeft = false;
							bNothingToProcess = false;
						}
					}
					if (bNothingToProcess) ___sleep(1);
				}
				if (!bStringsLeft) break;

				StringList* NewAlternated = new StringList();
				NewAlternated->allocate_string(testConfig.stringLen);
				memcpy(NewAlternated->string, LastProcessedProduced->string, LastProcessedProduced->len);

				StringAlternate(NewAlternated->string, NewAlternated->len);
				bNothingToProcess = true;
				if (AlternatedPtr != nullptr)
				{
					AlternatedPtr->next = NewAlternated;
					AlternatedPtr = AlternatedPtr->next;
				}
				else
				{
					AlternatedStrings = NewAlternated;
					AlternatedPtr = NewAlternated;
				}
			}
		});
	}
	Out.BeginResourceCleanup();
	
	delete ProducedStrings;
	delete UpperStrings;
	delete LowerStrings;
	delete AlternatedStrings;
	
	Out.EndTask(true);
}

#include "ParallelLib/ParallelLib.h"
void StringTest::DoParallelLib(const TestParams& In, RetryResult& Out)
{
	Out.BeginResourceInit();
	StringList* ProducedStrings = nullptr;
	StringList* UpperStrings = nullptr;
	StringList* LowerStrings = nullptr;
	StringList* AlternatedStrings = nullptr;
	std::mutex use_producer_object;

	bool bProductionCompleted = false;

	Out.BeginParallelWorkload();

	parallel_sections(stringSections, exec_master(no) nowait(no),
		parallel_section
		{
			StringList* ProducedPtr = nullptr;
			for (int i = 0; i < testConfig.numStringsToGenerate; ++i)
			{
				StringList* NewProduced = new StringList();
				NewProduced->allocate_string(testConfig.stringLen);
				GenerateRandomString(NewProduced->string, NewProduced->len);

				{
					const std::lock_guard<std::mutex> lock(use_producer_object);
					if (ProducedPtr != nullptr)
					{
						ProducedPtr->next = NewProduced;
						ProducedPtr = ProducedPtr->next;
					}
					else
					{
						ProducedStrings = NewProduced;
						ProducedPtr = NewProduced;
					}
				}
			}
			bProductionCompleted = true;
		}
		parallel_section
		{
			StringList* UpperPtr = nullptr;
			StringList* LastProcessedProduced = nullptr;
			bool bNothingToProcess = true;
			bool bStringsLeft = true;
			while (bStringsLeft)
			{
				while (bNothingToProcess && bStringsLeft)
				{
					{
						const std::lock_guard<std::mutex> lock(use_producer_object);
						//we haven't consumed anything yet but there is something to consume
						if (LastProcessedProduced == nullptr && ProducedStrings != nullptr)
						{
							LastProcessedProduced = ProducedStrings;
							bNothingToProcess = false;
						}
						//is there next item to consume?
						else if (LastProcessedProduced != nullptr && LastProcessedProduced->next != nullptr)
						{
							LastProcessedProduced = LastProcessedProduced->next;
							bNothingToProcess = false;
						}
						//no more items and producer claims it's finished
						else if (bProductionCompleted)
						{
							bStringsLeft = false;
							bNothingToProcess = false;
						}
					}
					if (bNothingToProcess) PARALLEL_SLEEP_MILISECONDS(1);
				}
				if (!bStringsLeft) break;

				StringList* NewUpper = new StringList();
				NewUpper->allocate_string(testConfig.stringLen);
				memcpy(NewUpper->string, LastProcessedProduced->string, LastProcessedProduced->len);
				StringToUpper(NewUpper->string, NewUpper->len);
				bNothingToProcess = true;
				if (UpperPtr != nullptr)
				{
					UpperPtr->next = NewUpper;
					UpperPtr = UpperPtr->next;
				}
				else
				{
					UpperStrings = NewUpper;
					UpperPtr = NewUpper;
				}
			}
		}
		parallel_section
		{
			StringList* LowerPtr = nullptr;
			StringList* LastProcessedProduced = nullptr;
			bool bNothingToProcess = true;
			bool bStringsLeft = true;
			while (bStringsLeft)
			{
				while (bNothingToProcess && bStringsLeft)
				{
					{
						const std::lock_guard<std::mutex> lock(use_producer_object);
						//we haven't consumed anything yet but there is something to consume
						if (LastProcessedProduced == nullptr && ProducedStrings != nullptr)
						{
							LastProcessedProduced = ProducedStrings;
							bNothingToProcess = false;
						}
						//is there next item to consume?
						else if (LastProcessedProduced != nullptr && LastProcessedProduced->next != nullptr)
						{
							LastProcessedProduced = LastProcessedProduced->next;
							bNothingToProcess = false;
						}
						//no more items and producer claims it's finished
						else if (bProductionCompleted)
						{
							bStringsLeft = false;
							bNothingToProcess = false;
						}
					}
					if (bNothingToProcess) PARALLEL_SLEEP_MILISECONDS(1);
				}
				if (!bStringsLeft) break;

				StringList* NewLower = new StringList();
				NewLower->allocate_string(testConfig.stringLen);
				memcpy(NewLower->string, LastProcessedProduced->string, LastProcessedProduced->len);

				StringToLower(NewLower->string, NewLower->len);
				bNothingToProcess = true;
				if (LowerPtr != nullptr)
				{
					LowerPtr->next = NewLower;
					LowerPtr = LowerPtr->next;
				}
				else
				{
					LowerStrings = NewLower;
					LowerPtr = NewLower;
				}
			}
		}
		parallel_section
		{
			StringList* AlternatedPtr = nullptr;
			StringList* LastProcessedProduced = nullptr;
			bool bNothingToProcess = true;
			bool bStringsLeft = true;
			while (bStringsLeft)
			{
				while (bNothingToProcess && bStringsLeft)
				{
					{
						const std::lock_guard<std::mutex> lock(use_producer_object);
						//we haven't consumed anything yet but there is something to consume
						if (LastProcessedProduced == nullptr && ProducedStrings != nullptr)
						{
							LastProcessedProduced = ProducedStrings;
							bNothingToProcess = false;
						}
						//is there next item to consume?
						else if (LastProcessedProduced != nullptr && LastProcessedProduced->next != nullptr)
						{
							LastProcessedProduced = LastProcessedProduced->next;
							bNothingToProcess = false;
						}
						//no more items and producer claims it's finished
						else if (bProductionCompleted)
						{
							bStringsLeft = false;
							bNothingToProcess = false;
						}
					}
					if (bNothingToProcess) PARALLEL_SLEEP_MILISECONDS(1);
				}
				if (!bStringsLeft) break;

				StringList* NewAlternated = new StringList();
				NewAlternated->allocate_string(testConfig.stringLen);
				memcpy(NewAlternated->string, LastProcessedProduced->string, LastProcessedProduced->len);

				StringAlternate(NewAlternated->string, NewAlternated->len);
				bNothingToProcess = true;
				if (AlternatedPtr != nullptr)
				{
					AlternatedPtr->next = NewAlternated;
					AlternatedPtr = AlternatedPtr->next;
				}
				else
				{
					AlternatedStrings = NewAlternated;
					AlternatedPtr = NewAlternated;
				}
			}
		}
	);

	Out.BeginResourceCleanup();
	
	delete ProducedStrings;
	delete UpperStrings;
	delete LowerStrings;
	delete AlternatedStrings;
	
	Out.EndTask(true);
}
