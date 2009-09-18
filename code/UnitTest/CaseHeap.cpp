#include "UnitTest/CaseHeap.h"
#include "Core/Heap/Heap.h"
#include "Core/Heap/GcNew.h"
#include "Core/Heap/Ref.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/Atomic.h"
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
		Heap::getInstance().collect();
	}
};

class HeapObject : public Object
{
public:
	static int32_t ms_instanceCount;

	HeapObject() { Atomic::increment(ms_instanceCount); }
	virtual ~HeapObject() { Atomic::decrement(ms_instanceCount); }
};

int32_t HeapObject::ms_instanceCount = 0;
int32_t g_nullPointerCount = 0;
int32_t g_allocCount = 0;
int32_t g_collectCount = 0;

void threadStressAlloc()
{
	Ref< HeapObject > object;
	while (!ThreadManager::getInstance().getCurrentThread()->stopped())
	{
		object = gc_new< HeapObject >();
		if (!object)
			g_nullPointerCount++;
		g_allocCount++;
		ThreadManager::getInstance().getCurrentThread()->sleep(0);
	}
}

void threadStressCollect()
{
	while (!ThreadManager::getInstance().getCurrentThread()->stopped())
	{
		Heap::getInstance().collect();
		g_collectCount++;
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
		Ref< HeapObject > foo1 = gc_new< HeapObject >();

		{
			Ref< HeapObject > foo2 = gc_new< HeapObject >();
			Ref< HeapObject > foo3 = gc_new< HeapObject >();

			CASE_ASSERT(foo1);
			CASE_ASSERT(foo2);
			CASE_ASSERT(foo3);

			log::info << HeapObject::ms_instanceCount << Endl;

			CASE_ASSERT_EQUAL(HeapObject::ms_instanceCount, 3);
		}

		ThreadManager::getInstance().getCurrentThread()->sleep(1000);

		Heap::getInstance().collect();
		Heap::getInstance().collect();
		Heap::getInstance().collect();

		log::info << HeapObject::ms_instanceCount << Endl;

		CASE_ASSERT(foo1);
		CASE_ASSERT_EQUAL(HeapObject::ms_instanceCount, 1);

		foo1 = 0;

		ThreadManager::getInstance().getCurrentThread()->sleep(1000);

		Heap::getInstance().collect();
		Heap::getInstance().collect();
		Heap::getInstance().collect();

		log::info << HeapObject::ms_instanceCount << Endl;

		CASE_ASSERT_EQUAL(HeapObject::ms_instanceCount, 0);
	}

	{
		Thread* threadAlloc = ThreadManager::getInstance().create(makeStaticFunctor(&threadStressAlloc));
		Thread* threadCollect = ThreadManager::getInstance().create(makeStaticFunctor(&threadStressCollect));

		threadAlloc->start();
		threadCollect->start();

		ThreadManager::getInstance().getCurrentThread()->sleep(5000);

		threadAlloc->stop();
		threadCollect->stop();

		ThreadManager::getInstance().destroy(threadAlloc);
		ThreadManager::getInstance().destroy(threadCollect);

		log::info << g_allocCount << L" allocated objects" << Endl;
		log::info << g_collectCount << L" forced garbage collects" << Endl;

		CASE_ASSERT_EQUAL(g_nullPointerCount, 0);

		ThreadManager::getInstance().getCurrentThread()->sleep(2000);

		for (int i = 0; i < 100; ++i)
			Heap::getInstance().collect();

		log::info << HeapObject::ms_instanceCount << Endl;

		CASE_ASSERT_EQUAL(HeapObject::ms_instanceCount, 0);

		Heap::getInstance().collect();
	}
}

}
