#pragma once
#include "Test.h"

struct StringList
{
	StringList* next = nullptr;
	char* string = nullptr;
	int len = 0;

	void allocate_string(int targetLen)
	{
		len = targetLen;
		string = new char[len];
	}

	~StringList()
	{
		if (next != nullptr)
		{
			delete next;
		}
		delete[] string;
	}
};

struct StringTestConfig
{
	const int numStringsToGenerate = 100;
	const int stringLen = 1024;
};

class StringTest : public Test
{
private:
	static void GenerateRandomString(char* buffer, int len);
	static void StringToUpper(char* buffer, int len);
	static void StringToLower(char* buffer, int len);
	static void StringAlternate(char* buffer, int len);

	StringTestConfig testConfig;

public:
	StringTest(const std::string& inName, const StringTestConfig& config);
	~StringTest();

protected:
	virtual void DoSequentially(const TestParams& In, RetryResult& Out) override;
	virtual void DoParallelLib(const TestParams& In, RetryResult& Out) override;
	virtual void DoOpenMP(const TestParams& In, RetryResult& Out) override;
	virtual void DoTBB(const TestParams& In, RetryResult& Out) override;
	virtual void DoDlib(const TestParams& In, RetryResult& Out) override;
};

