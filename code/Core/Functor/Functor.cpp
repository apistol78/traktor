#include "Core/Functor/Functor.h"
#include "Core/Singleton/ISingleton.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Memory/Alloc.h"
#include "Core/Memory/BlockAllocator.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Atomic.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/ThreadManager.h"

namespace traktor
{
	namespace
	{

class FunctorHeap : public ISingleton
{
public:
	static FunctorHeap& getInstance()
	{
		static FunctorHeap* s_instance = 0;
		if (!s_instance)
		{
			s_instance = new FunctorHeap();
			SingletonManager::getInstance().addAfter(s_instance, &ThreadManager::getInstance());
		}
		return *s_instance;
	}

	void* alloc(uint32_t size)
	{
		void* ptr = 0;
		
		if (size <= MaxFunctorSize)
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_allocatorLock);
			ptr = m_blockAllocator.alloc();
		}

		if (!ptr)
		{
			ptr = Alloc::acquireAlign(size, 16, T_FILE_LINE);
			if (!ptr)
				T_FATAL_ERROR;
		}

#if defined(_DEBUG)
		m_count++;
#endif
		return ptr;
	}

	void free(void* ptr)
	{
		if (!ptr)
			return;

		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_allocatorLock);

		bool result = m_blockAllocator.free(ptr);
		if (!result)
			Alloc::freeAlign(ptr);

#if defined(_DEBUG)
		m_count--;
#endif
	}

protected:
	virtual void destroy() { delete this; }

private:
	enum { MaxFunctorCount = 8192+4096 };
	enum { MaxFunctorSize = 64 };

	void* m_block;
	BlockAllocator m_blockAllocator;
	Semaphore m_allocatorLock;
#if defined(_DEBUG)
	int32_t m_count;
#endif

	FunctorHeap()
	:	m_block(Alloc::acquireAlign(MaxFunctorCount * MaxFunctorSize, 16, T_FILE_LINE))
	,	m_blockAllocator(m_block, MaxFunctorCount, MaxFunctorSize)
#if defined(_DEBUG)
	,	m_count(0)
#endif
	{
	}

	virtual ~FunctorHeap()
	{
		T_ASSERT_M (m_count == 0, L"There are still functors allocated, memory leak?");
		Alloc::freeAlign(m_block);
	}
};

	}

void* Functor::operator new (size_t size)
{
	return FunctorHeap::getInstance().alloc(uint32_t(size));
}

void Functor::operator delete (void* ptr)
{
	FunctorHeap::getInstance().free(ptr);
}

}
