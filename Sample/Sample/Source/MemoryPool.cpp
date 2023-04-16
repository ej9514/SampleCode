#include <Windows.h>

#include "MemoryPool.h"

EJ::Chunk::Chunk(int32_t dataSizeType, int32_t dataSize)
{
	m_sizeType = dataSizeType;

	for (int32_t i = 0; i < g_dataCountInChunk; ++i)
	{
		m_data[i] = (int8_t*)malloc(sizeof(DataHeader) + dataSize);
		if (m_data[i] != nullptr)
		{
			DataHeader* header = (DataHeader*)m_data[i];
			*header = (DataHeader)this;
		}
	}
}

EJ::Chunk::~Chunk()
{
	for (int32_t i = 0; i < g_dataCountInChunk; ++i)
	{
		if (m_data[i] != nullptr)
		{
			free(m_data[i]);
		}
	}
}

EJ::ChunkPool::ChunkPool()
{
}

EJ::ChunkPool::~ChunkPool()
{
	Reset();
}

bool EJ::ChunkPool::Init(uint32_t chunkCount, uint32_t dataSize, uint32_t dataSizeType)
{
	Reset();

	if (dataSize <= 0 || dataSizeType < 0)
		return false;

	m_dataSize = dataSize;
	m_dataSizeType = dataSizeType;

	if (chunkCount <= 0)
		chunkCount = g_defaultPoolSize;

	m_chunkCount = chunkCount;

	for (uint32_t i = 0; i < m_chunkCount; ++i)
	{
		Chunk* newData = new Chunk(m_dataSizeType, m_dataSize);
		if (newData == nullptr)
			continue;

		newData->m_allocCount = 0;
		newData->m_freeCount = 0;

		m_pool.push(newData);
	}

	return true;
}

void EJ::ChunkPool::Reset()
{
	Chunk* data = nullptr;
	while (m_pool.try_pop(data) == true)
	{
		if (data == nullptr)
			continue;

		delete data;
	}
	m_chunkCount = 0;
}

EJ::Chunk* EJ::ChunkPool::Alloc()
{
	if (m_dataSize <= 0 || m_dataSizeType < 0)
		return nullptr;

	Chunk* chunk = nullptr;
	if (m_pool.try_pop(chunk) == false)
	{
		chunk = new Chunk(m_dataSizeType, m_dataSize);
		InterlockedIncrement(&m_chunkCount);
	}

	if (chunk != nullptr)
	{
		chunk->m_allocCount = 0;
		chunk->m_freeCount = 0;
	}

	return chunk;
}

void EJ::ChunkPool::Free(Chunk* data)
{
	if (data == nullptr)
		return;

	m_pool.push(data);
}


EJ::MemoryPool::MemoryPool()
{
	m_TLS_Index = TlsAlloc();
	if (m_TLS_Index == TLS_OUT_OF_INDEXES)
		return;

	uint32_t dataSize[g_dataSizeTypeCount] = { 0, };
	m_increaseDataSize = g_dataMaxSize / g_dataSizeTypeCount;
	for (int32_t sizeType = 0; sizeType < g_dataSizeTypeCount; ++sizeType)
	{
		dataSize[sizeType] = m_increaseDataSize * (sizeType + 1);
	}
	dataSize[g_dataSizeTypeCount - 1] = g_dataMaxSize;

	for (int32_t sizeType = 0; sizeType < g_dataSizeTypeCount; ++sizeType)
	{
		m_pool[sizeType].Init(g_defaultChunkCount, dataSize[sizeType], sizeType);
	}
}

EJ::MemoryPool::~MemoryPool()
{
}

int32_t EJ::MemoryPool::GetSizeType(uint32_t dataSize)
{
	if (dataSize > g_dataMaxSize || dataSize == 0)
		return -1;

	int32_t sizeType = dataSize / m_increaseDataSize;
	if (dataSize % m_increaseDataSize == 0)
		sizeType -= 1;

	return sizeType;
}

int8_t* EJ::MemoryPool::Alloc(uint32_t allocDataSize)
{
	int32_t sizeType = GetSizeType(allocDataSize);
	if (sizeType < 0)
		return nullptr;

	TlsValue* tlsValue = (TlsValue*)TlsGetValue(m_TLS_Index);
	if (tlsValue == nullptr)
	{
		tlsValue = new TlsValue();
		TlsSetValue(m_TLS_Index, tlsValue);
	}

	Chunk* chunk = tlsValue->m_chunk[sizeType];
	if (chunk == nullptr)
	{
		chunk = m_pool[sizeType].Alloc();
		tlsValue->m_chunk[sizeType] = chunk;
	}

	int8_t* ptr = chunk->m_data[chunk->m_allocCount++] + sizeof(DataHeader);
	if (chunk->m_allocCount == g_dataCountInChunk)
	{
		tlsValue->m_chunk[sizeType] = nullptr;
	}

	return ptr;
}

void EJ::MemoryPool::Free(void* data)
{
	if (data == nullptr)
		return;

	DataHeader* header = (DataHeader*)((int8_t*)data - sizeof(DataHeader));
	Chunk* chunk = (Chunk*)(*header);
	chunk->m_freeCount += 1;
	if (chunk->m_freeCount == g_dataCountInChunk)
	{
		if (chunk->m_sizeType < 0 || chunk->m_sizeType >= g_dataSizeTypeCount)
			return;

		m_pool[chunk->m_sizeType].Free(chunk);
	}
}