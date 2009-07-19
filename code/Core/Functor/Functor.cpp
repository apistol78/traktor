#include "Core/Functor/Functor.h"
#include "Core/Singleton/Singleton.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Heap/Heap.h"
#include "Core/Heap/Alloc.h"
#include "Core/Heap/BlockAllocator.h"
#include "Core/Thread/CriticalSection.h"
#include "Core/Thread/Acquire.h"

namespace traktor
{
	namespace
	{

class FunctorHeap : public Singleton
{
	T_RTTI_CLASS(FunctorHeap)

public:
	static FunctorHeap& getInstance()
	{
		static FunctorHeap* s_instance = 0;
		if (!s_instance)
		{
			s_instance = new FunctorHeap();
			SingletonManager::getInstance().addAfter(s_instance, &Heap::getInstance());
		}
		return *s_instance;
	}

	void* alloc(uint32_t size)
	{
		T_ASSERT_M (size <= MaxFunctorSize, L"Allocation size too big");
		Acquire< CriticalSection > __lock__(m_allocatorLock);
		void* ptr = m_blockAllocator.alloc();
		T_ASSERT_M (ptr, L"Out of memory");
#if defined(_DEBUG)
		++m_count;
#endif
		return ptr;
	}

	void free(void* ptr)
	{
		if (!ptr)
			return;

		Acquire< CriticalSection > __lock__(m_allocatorLock);
		bool result = m_blockAllocator.free(ptr);
		T_ASSERT_M (result, L"Invalid pointer");
#if defined(_DEBUG)
		--m_count;
#endif
	}

protected:
	virtual void destroy() { delete this; }

private:
	enum { MaxFunctorCount = 1024 };
	enum { MaxFunctorSize = 64 };
	
	void* m_block;
	BlockAllocator m_blockAllocator;
	CriticalSection m_allocatorLock;
#if defined(_DEBUG)
	uint32_t m_count;
#endif

	FunctorHeap()
	:	m_block(Alloc::acquireAlign(MaxFunctorCount * MaxFunctorSize, 16))
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.FunctorHeap", FunctorHeap, Singleton)

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
