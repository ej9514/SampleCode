#pragma once

#include <Windows.h>
#include <thread>
#include <vector>
#include <chrono>
#include <functional>

#include "MemoryPool.h"
#include "ObjectPool.h"

using namespace std::chrono;

namespace EJ
{
	class PoolTest
	{
		using TestFunction = std::function<void(HANDLE)>;

	public:
		static void Test_Performance();
		static void Test_Verify_Operation();

	private:
		// TestClass
		class PoolTestClass01
		{
		public:
			int8_t m_arr[2000];
		};

		class PoolTestClass02
		{
		public:
			static const int32_t m_testValue01 = 0x11111111;
			static const int32_t m_testValue02 = 0xEEEEEEEE;

			int32_t m_value = m_testValue01;
		};

		static void TestCase(HANDLE startEvent, const int32_t threadCount, std::function<void()> func);
	};

	void PoolTest::Test_Performance()
	{
		HANDLE startEvent = CreateEvent(NULL, true, false, NULL);
		if (startEvent == 0)
			return;

		constexpr int32_t threadCount = 4;

		constexpr int32_t loopCount = 10000000;
		constexpr int32_t allocLoopCount = 10;
		constexpr int32_t testDataSize = sizeof(PoolTestClass01);

		printf("\nObject Pool Test...\n");
		ObjectPool<PoolTestClass01> ObjectPool;
		TestCase(startEvent, threadCount, [&ObjectPool, startEvent, loopCount, allocLoopCount, testDataSize]() ->void
			{
				PoolTestClass01* arr[allocLoopCount] = { 0, };
				WaitForSingleObject(startEvent, INFINITE);

				for (int32_t i = 0; i < loopCount; ++i)
				{
					for (int32_t alloc = 0; alloc < allocLoopCount; ++alloc)
					{
						arr[alloc] = ObjectPool.New();
					}

					for (int32_t deAlloc = 0; deAlloc < allocLoopCount; ++deAlloc)
					{
						ObjectPool.Delete(arr[deAlloc]);
					}
				}
			});

		printf("\nMemory Pool Test...\n");
		MemoryPool memoryPool;
		TestCase(startEvent, threadCount, [&memoryPool, startEvent, loopCount, allocLoopCount, testDataSize]() ->void
			{
				PoolTestClass01* arr[allocLoopCount] = { 0, };
				WaitForSingleObject(startEvent, INFINITE);

				for (int32_t i = 0; i < loopCount; ++i)
				{
					for (int32_t alloc = 0; alloc < allocLoopCount; ++alloc)
					{
						arr[alloc] = (PoolTestClass01*)memoryPool.Alloc(testDataSize);
					}

					for (int32_t deAlloc = 0; deAlloc < allocLoopCount; ++deAlloc)
					{
						memoryPool.Free(arr[deAlloc]);
					}
				}
			});

		printf("\nNew/Delete Test...\n");
		TestCase(startEvent, threadCount, [startEvent, loopCount, allocLoopCount, testDataSize]() ->void
			{
				PoolTestClass01* arr[allocLoopCount] = { 0, };
				WaitForSingleObject(startEvent, INFINITE);

				for (int32_t i = 0; i < loopCount; ++i)
				{
					for (int32_t alloc = 0; alloc < allocLoopCount; ++alloc)
					{
						arr[alloc] = new PoolTestClass01;
					}

					for (int32_t deAlloc = 0; deAlloc < allocLoopCount; ++deAlloc)
					{
						delete arr[deAlloc];
					}
				}
			});


		return;
	}

	void PoolTest::Test_Verify_Operation()
	{
		printf("\nNew/Delete Test...\n");
		HANDLE startEvent = CreateEvent(NULL, true, false, NULL);
		if (startEvent == 0)
			return;

		constexpr int32_t threadCount = 8;

		constexpr int32_t loopCount = 10000000;
		constexpr int32_t allocLoopCount = 10;
		constexpr int32_t testDataSize = sizeof(PoolTestClass02);

		printf("\nVerify_Operation...\n");
		ObjectPool<PoolTestClass02> objectPool;

		TestCase(startEvent, threadCount, [&objectPool, startEvent, loopCount, allocLoopCount, testDataSize]() ->void
			{
				PoolTestClass02* arr[allocLoopCount] = { 0, };
				WaitForSingleObject(startEvent, INFINITE);

				for (int32_t i = 0; i < loopCount; ++i)
				{
					for (int32_t alloc = 0; alloc < allocLoopCount; ++alloc)
					{
						arr[alloc] = objectPool.New();
						if (arr[alloc] == nullptr)
						{
							std::cout << "alloc data is nullptr..." << std::endl;
							return;
						}

						if (arr[alloc]->m_value != PoolTestClass02::m_testValue01)
						{
							std::cout << "Invalid value..." << std::endl;
							return;
						}

						arr[alloc]->m_value = PoolTestClass02::m_testValue02;
					}

					for (int32_t deAlloc = 0; deAlloc < allocLoopCount; ++deAlloc)
					{
						if (arr[deAlloc] == nullptr)
						{
							std::cout << "dealloc data is nullptr..." << std::endl;
							return;
						}

						if (arr[deAlloc]->m_value != PoolTestClass02::m_testValue02)
						{
							std::cout << "Invalid value..." << std::endl;
							return;
						}

						arr[deAlloc]->m_value = PoolTestClass02::m_testValue01;

						objectPool.Delete(arr[deAlloc]);
					}
				}
			});

		printf("\nSuccess.\n");
		return;
	}

	void PoolTest::TestCase(HANDLE startEvent, const int32_t threadCount, std::function<void()> func)
	{
		ResetEvent(startEvent);

		std::vector<std::thread> vecThread;
		for (int32_t i = 0; i < threadCount; ++i)
		{
			vecThread.push_back(std::thread(func));
		}

		system_clock::time_point logicStartTIme = system_clock::now();

		SetEvent(startEvent);

		for (auto& data : vecThread)
		{
			data.join();
		}

		system_clock::time_point logicEndTime = system_clock::now();

		milliseconds logicTime = duration_cast<milliseconds>(logicEndTime - logicStartTIme);

		printf("[LogicTime: %lld]\n", logicTime.count());

		return;
	}
}
