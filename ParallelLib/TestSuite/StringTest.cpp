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

	StringList ProducedStrings, UpperStrings, LowerStrings, AlternatedStrings;
	StringList* ProducedPtr   = &ProducedStrings;
	StringList* UpperPtr      = &UpperStrings;
	StringList* LowerPtr      = &LowerStrings;
	StringList* AlternatedPtr = &AlternatedStrings;

	Out.BeginParallelWorkload();

	for (int i = 0; i < testConfig.numStringsToGenerate; ++i)
	{
		StringList* NewProduced = new StringList();
		NewProduced->allocate_string(testConfig.stringLen);
		GenerateRandomString(NewProduced->string, NewProduced->len);
		ProducedPtr->next = NewProduced;
		ProducedPtr = ProducedPtr->next;
	}

	StringList* LastProcessedProduced = ProducedPtr;
	do
	{
		StringList* NewUpper = new StringList();
		NewUpper->allocate_string(testConfig.stringLen);
		memcpy(NewUpper->string, LastProcessedProduced->string, LastProcessedProduced->len);
		StringToUpper(NewUpper->string, NewUpper->len);
		UpperPtr->next = NewUpper;
		UpperPtr = UpperPtr->next;
	}
	while ((LastProcessedProduced = LastProcessedProduced->next) != nullptr);

	LastProcessedProduced = ProducedPtr;
	do
	{
		StringList* NewLower = new StringList();
		NewLower->allocate_string(testConfig.stringLen);
		memcpy(NewLower->string, LastProcessedProduced->string, LastProcessedProduced->len);

		StringToLower(NewLower->string, NewLower->len);
		LowerPtr->next = NewLower;
		LowerPtr = LowerPtr->next;
	}
	while ((LastProcessedProduced = LastProcessedProduced->next) != nullptr);

	LastProcessedProduced = ProducedPtr;
	do
	{
		StringList* NewAlternated = new StringList();
		NewAlternated->allocate_string(testConfig.stringLen);
		memcpy(NewAlternated->string, LastProcessedProduced->string, LastProcessedProduced->len);

		StringAlternate(NewAlternated->string, NewAlternated->len);
		AlternatedPtr->next = NewAlternated;
		AlternatedPtr = AlternatedPtr->next;
	}
	while ((LastProcessedProduced = LastProcessedProduced->next) != nullptr);

	Out.BeginResourceCleanup();

	printf("\n\n%s\n%s\n%s\n%s\n\n", ProducedStrings.next->string, UpperStrings.next->string, LowerStrings.next->string, AlternatedStrings.next->string);
	ProducedStrings.remove_chain();
	UpperStrings.remove_chain();
	LowerStrings.remove_chain();
	AlternatedStrings.remove_chain();

	Out.EndTask(true);
}

void StringTest::DoParallelLib(const TestParams& In, RetryResult& Out)
{
	throw;
}

void StringTest::DoOpenMP(const TestParams& In, RetryResult& Out)
{
	throw;
}

void StringTest::DoTBB(const TestParams& In, RetryResult& Out)
{
	throw;
}

void StringTest::DoDlib(const TestParams& In, RetryResult& Out)
{
	throw;
}
