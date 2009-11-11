#include "UnitTest/CaseHeap.h"
#include "Core/Heap/Heap.h"
#include "Core/Heap/GcNew.h"
#include "Core/Heap/Ref.h"
#include "Core/Heap/RefArray.h"
#include "Core/Thread/Atomic.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace
	{

class LengthyCtorObject : Object
{
public:
	LengthyCtorObject()
	{
		ThreadManager::getInstance().getCurrentThread()->sleep(1000);
		Heap::collect();
	}
};

class HeapObject : public Object
{
public:
	static int32_t ms_instanceCount;
	int32_t m_index;

	HeapObject(int32_t index) : m_index(index) { Atomic::increment(ms_instanceCount); }
	virtual ~HeapObject() { m_index = -1; Atomic::decrement(ms_instanceCount); }
};

int32_t HeapObject::ms_instanceCount = 0;
int32_t g_nullPointerCount = 0;
int32_t g_indexErrorCount = 0;
int32_t g_allocCount = 0;
int32_t g_collectCount = 0;

void threadStressAlloc()
{
	Ref< HeapObject > object;
	while (!ThreadManager::getInstance().getCurrentThread()->stopped())
	{
		object = gc_new< HeapObject >(0);
		if (!object)
			Atomic::increment(g_nullPointerCount);
		Atomic::increment(g_allocCount);
		ThreadManager::getInstance().getCurrentThread()->sleep(0);
	}
}

void threadStressAllocArray()
{
	while (!ThreadManager::getInstance().getCurrentThread()->stopped())
	{
		RefArray< HeapObject > objects;
		for (int i = 0; i < 100; ++i)
			objects.push_back(gc_new< HeapObject >(i));

		Atomic::increment(g_allocCount);

		ThreadManager::getInstance().getCurrentThread()->sleep(0);

		for (int i = 0; i < 100; ++i)
		{
			if (objects[i])
			{
				if (objects[i]->m_index != i)
					Atomic::increment(g_indexErrorCount);
			}
			else
				Atomic::increment(g_nullPointerCount);
		}
	}	
}

void threadStressCollect()
{
	while (!ThreadManager::getInstance().getCurrentThread()->stopped())
	{
		Heap::collect();
		Atomic::increment(g_collectCount);
		ThreadManager::getInstance().getCurrentThread()->sleep(10);
	}
}

	}

void CaseHeap::run()
{
	{
		Ref< LengthyCtorObject > foo = gc_new< LengthyCtorObject >();
		CASE_ASSERT(foo);
	}

	{
		Ref< HeapObject > foo1 = gc_new< HeapObject >(0);

		{
			Ref< HeapObject > foo2 = gc_new< HeapObject >(1);
			Ref< HeapObject > foo3 = gc_new< HeapObject >(2);

			CASE_ASSERT(foo1);
			CASE_ASSERT(foo2);
			CASE_ASSERT(foo3);

			log::info << HeapObject::ms_instanceCount << Endl;

			CASE_ASSERT_EQUAL(HeapObject::ms_instanceCount, 3);
		}

		ThreadManager::getInstance().getCurrentThread()->sleep(1000);

		Heap::collect();
		Heap::collect();
		Heap::collect();

		log::info << HeapObject::ms_instanceCount << Endl;

		CASE_ASSERT(foo1);
		CASE_ASSERT_EQUAL(HeapObject::ms_instanceCount, 1);

		foo1 = 0;

		ThreadManager::getInstance().getCurrentThread()->sleep(1000);

		Heap::collect();
		Heap::collect();
		Heap::collect();

		log::info << HeapObject::ms_instanceCount << Endl;

		CASE_ASSERT_EQUAL(HeapObject::ms_instanceCount, 0);
	}

	{
		Thread* threads[8];

		for (int i = 0; i < 6; ++i)
			threads[i] = ThreadManager::getInstance().create(makeStaticFunctor(&threadStressAlloc));
		for (int i = 6; i < 8; ++i)
			threads[i]= ThreadManager::getInstance().create(makeStaticFunctor(&threadStressCollect));

		for (int i = 0; i < 8; ++i)
			threads[i]->start();

		ThreadManager::getInstance().getCurrentThread()->sleep(10000);

		for (int i = 0; i < 8; ++i)
			threads[i]->stop();

		for (int i = 0; i < 8; ++i)
			ThreadManager::getInstance().destroy(threads[i]);

		log::info << g_allocCount << L" allocated objects" << Endl;
		log::info << g_collectCount << L" forced garbage collects" << Endl;

		CASE_ASSERT_EQUAL(g_nullPointerCount, 0);

		// Wait until orphan objects get promoted to collect-able.
		ThreadManager::getInstance().getCurrentThread()->sleep(11000);

		for (int i = 0; i < 10; ++i)
			Heap::collect();

		ThreadManager::getInstance().getCurrentThread()->sleep(1000);

		log::info << HeapObject::ms_instanceCount << Endl;
		CASE_ASSERT_EQUAL(HeapObject::ms_instanceCount, 0);
	}

	g_allocCount = 0;
	g_collectCount = 0;

	{
		Thread* threads[8];

		for (int i = 0; i < 6; ++i)
			threads[i] = ThreadManager::getInstance().create(makeStaticFunctor(&threadStressAllocArray));
		for (int i = 6; i < 8; ++i)
			threads[i]= ThreadManager::getInstance().create(makeStaticFunctor(&threadStressCollect));

		for (int i = 0; i < 8; ++i)
			threads[i]->start();

		ThreadManager::getInstance().getCurrentThread()->sleep(10000);

		for (int i = 0; i < 8; ++i)
			threads[i]->stop();

		for (int i = 0; i < 8; ++i)
			ThreadManager::getInstance().destroy(threads[i]);

		log::info << g_allocCount << L" allocated arrays" << Endl;
		log::info << g_collectCount << L" forced garbage collects" << Endl;

		// Wait until orphan objects get promoted to collect-able.
		ThreadManager::getInstance().getCurrentThread()->sleep(11000);

		for (int i = 0; i < 10; ++i)
			Heap::collect();

		ThreadManager::getInstance().getCurrentThread()->sleep(1000);

		log::info << HeapObject::ms_instanceCount << Endl;
		CASE_ASSERT_EQUAL(HeapObject::ms_instanceCount, 0);

		CASE_ASSERT_EQUAL(g_nullPointerCount, 0);
		CASE_ASSERT_EQUAL(g_indexErrorCount, 0);
	}
}

}
