#pragma once
#include <iostream>
#include <concurrent_queue.h>

#include "MemoryPool.h"

namespace EJ
{
	template<class T>
	class ObjectPool
	{
	private:
		uint32_t m_TLS_Index = 0;

		ChunkPool m_pool;
	public:
		ObjectPool();
		virtual	~ObjectPool();

		T* New();
		void Delete(T* object);
	};

	template<class T>
	ObjectPool<T>::ObjectPool()
	{
		m_TLS_Index = TlsAlloc();
		if (m_TLS_Index == TLS_OUT_OF_INDEXES)
			return;

		m_pool.Init(g_defaultChunkCount, sizeof(T), 0);
	}

	template<class T>
	ObjectPool<T>::~ObjectPool()
	{
	}

	template<class T>
	T* ObjectPool<T>::New()
	{
		Chunk* chunk = (Chunk*)TlsGetValue(m_TLS_Index);
		if (chunk == nullptr)
		{
			chunk = m_pool.Alloc();
			TlsSetValue(m_TLS_Index, chunk);
		}

		T* ptr = (T*)(chunk->m_data[chunk->m_allocCount++] + sizeof(DataHeader));
		if (chunk->m_allocCount == g_dataCountInChunk)
		{
			chunk = nullptr;
			TlsSetValue(m_TLS_Index, nullptr);
		}

		if (ptr == nullptr)
			return nullptr;

		return new(ptr) T; // placement new
	}

	template<class T>
	void ObjectPool<T>::Delete(T* object)
	{
		if (object == nullptr)
			return;

		object->~T(); // ¼Ò¸êÀÚ È£Ãâ

		DataHeader* header = (DataHeader*)((int8_t*)object - sizeof(DataHeader));
		Chunk* chunk = (Chunk*)(*header);
		chunk->m_freeCount += 1;
		if (chunk->m_freeCount == g_dataCountInChunk)
		{
			if (chunk->m_sizeType < 0 || chunk->m_sizeType >= g_dataSizeTypeCount)
				return;

			m_pool.Free(chunk);
		}
	}
}