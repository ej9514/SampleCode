#pragma once
#include <iostream>
#include <concurrent_queue.h>

namespace EJ
{
	using DataHeader = uint64_t;

	constexpr uint32_t g_defaultPoolSize = 100;
	constexpr uint32_t g_defaultChunkCount = 10;
	constexpr uint32_t g_dataCountInChunk = 200;
	constexpr uint32_t g_dataMaxSize = 10000;
	constexpr uint32_t g_dataSizeTypeCount = 100;

	class Interface_Pool
	{
	public:
		virtual int8_t* Alloc(uint32_t allocDataSize) = 0;
		virtual void Free(void* data) = 0;
	};

	class Chunk
	{
	public:
		uint32_t m_sizeType = 0;
		int8_t* m_data[g_dataCountInChunk];
		uint32_t m_allocCount = 0;
		uint32_t m_freeCount = 0;

		Chunk(int32_t dataSizeType, int32_t dataSize);
		~Chunk();
	};

	struct TlsValue
	{
		Chunk* m_chunk[g_dataSizeTypeCount] = {0,};
	};

	class ChunkPool
	{
	private:
		concurrency::concurrent_queue<Chunk*> m_pool;
		uint32_t m_chunkCount = 0;

		uint32_t m_dataSize = 0;
		uint32_t m_dataSizeType = 0;

	public:
		ChunkPool();
		~ChunkPool();

		bool Init(uint32_t chunkCount, uint32_t dataSize, uint32_t dataSizeType);
		void Reset();

		Chunk* Alloc();
		void Free(Chunk* data);
	};

	class MemoryPool : public Interface_Pool
	{
	private:
		uint32_t m_TLS_Index = 0;

		ChunkPool m_pool[g_dataSizeTypeCount];
		uint32_t m_increaseDataSize = 0;
	public:
		MemoryPool();
		virtual	~MemoryPool();

		int32_t GetSizeType(uint32_t dataSize);
		int8_t* Alloc(uint32_t allocDataSize) override;
		void Free(void* data) override;
	};
}