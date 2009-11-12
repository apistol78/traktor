#include <algorithm>
#include <list>
#include "Core/Containers/IntrusiveList.h"
#include "Core/Heap/Heap.h"
#include "Core/Heap/HeapConfig.h"
#include "Core/Heap/HeapStats.h"
#include "Core/Heap/StdAllocator.h"
#if defined(T_HEAP_FAST_ALLOCATOR)
#	include "Core/Heap/FastAllocator.h"
#endif
#if defined(T_HEAP_DEBUG_ALLOCATOR)
#	include "Core/Heap/DebugAllocator.h"
#endif
#include "Core/Heap/Ref.h"
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Atomic.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadLocal.h"
#include "Core/Thread/ThreadManager.h"

namespace traktor
{

const uint32_t c_magic = 'TKTR';
const int32_t c_eventsUntilCollect = 4000;

#pragma pack(1)

/*! \brief Object allocation information.
 *
 * Used by Heap to keep track of allocations and
 * collection flags.
 *
 * \note
 * Size of this structure must be a multiple of
 * maximum alignment, i.e. 16 bytes.
 */
struct ObjectHeader
{
	uint32_t m_magic;					//!< Magic signature; used to indicate object is allocated through heap.
	int32_t m_refCount;					//!< Object reference count.
	IntrusiveList< RefBase > m_crefs;	//!< Child reference containers; used to determine cycles.

	struct 
	{
		unsigned m_size : 24;
		unsigned m_buffered : 1;
		unsigned m_visited : 1;
		unsigned m_pending : 1;
		unsigned m_dead : 1;			//!< Object is dead; only valid if T_HEAP_DEBUG_KEEP_DEAD is defined.
		unsigned m_unused : 4;
	};

	ObjectHeader()
	:	m_magic(c_magic)
	,	m_refCount(0)
	,	m_size(0)
	,	m_buffered(0)
	,	m_visited(0)
	,	m_pending(0)
	,	m_dead(0)
	{
	}
};

#pragma pack()

#if !defined(_DEBUG)
	namespace
	{
#endif

typedef Semaphore lock_primitive_t;
typedef std::list< ObjectHeader* > object_list_t;
typedef std::list< ObjectHeader* > object_stack_t;

#if defined(T_HEAP_THREAD_SAFE)
#	define HEAP_LOCK Acquire< lock_primitive_t > __prvlock__(g_heapLock);
#else
#	define HEAP_LOCK
#endif

#if defined(T_HEAP_DEBUG_ALLOCATOR)
StdAllocator g_stdAllocator;
DebugAllocator g_allocator(&g_stdAllocator);
#elif defined(T_HEAP_FAST_ALLOCATOR)
StdAllocator g_stdAllocator;
FastAllocator g_allocator(&g_stdAllocator);
#else
StdAllocator g_allocator;
#endif

lock_primitive_t g_heapLock;
ThreadLocal g_creatingObjectStack;
IntrusiveList< RefBase > g_rootRefs;
object_list_t g_cycleObjects;
int32_t g_eventsUntilCollect = c_eventsUntilCollect;	//!< Number of events until cycle collect.
int32_t g_lockRefCount = 0;								//!< Number of locked references; not permitted to run cycle collector if non zero.
bool g_collectRunning = false;							//!< Flag indicate cycle collector running.
Semaphore g_lockRef;
HeapStats g_stats = { 0, 0, 0, 0 };

T_FORCE_INLINE size_t getAllocSize(size_t objectSize)
{
	const size_t c_objectHeaderSize = sizeof(ObjectHeader);
	return c_objectHeaderSize + objectSize;
}

T_FORCE_INLINE Object* getObject(ObjectHeader* header)
{
	T_ASSERT (header->m_dead == 0);
	return reinterpret_cast< Object* >(reinterpret_cast< uint8_t* >(header) + sizeof(ObjectHeader));
}

T_FORCE_INLINE ObjectHeader* getObjectHeader(Object* object)
{
	if (object)
	{
		ObjectHeader* header = reinterpret_cast< ObjectHeader* >(reinterpret_cast< uint8_t* >(object) - sizeof(ObjectHeader));
		if (header->m_magic == c_magic)
		{
			T_ASSERT (header->m_dead == 0);
			return header;
		}
	}
	return 0;
}

T_FORCE_INLINE void freeObject(ObjectHeader* header)
{
	T_ASSERT (header->m_dead == 0);
	T_ASSERT (header->m_buffered == 0);

	Object* object = getObject(header);
	object->~Object();

#if defined(T_HEAP_DEBUG_KEEP_DEAD)
	std::memset(object, 0, header->m_size);
	header->m_dead = 1;
#else
	g_allocator.free(header);
#endif

	Atomic::decrement(g_stats.objects);
}

#if !defined(_DEBUG)
	}
#endif

void* Heap::preConstructor(size_t size, size_t align)
{
	T_ASSERT_M (size > 0, L"Invalid size, zero");
	T_ASSERT_M (size < (1 << 24), L"Invalid size, too great");
	T_ASSERT_M (align <= 16, L"Alignment must be 16 or less");

	// Invoke cycle collector if we've reached thresholds.
	if (--g_eventsUntilCollect <= 0)
	{
		g_eventsUntilCollect = c_eventsUntilCollect;
		collect();
	}

	// Allocate object, including object header.
	void* ptr = g_allocator.alloc(getAllocSize(size), align);
	T_FATAL_ASSERT_M(ptr != 0, L"Out of memory");

	// Initialize object header.
	ObjectHeader* creatingObject = new (ptr) ObjectHeader();
	creatingObject->m_size = unsigned(size);

	// Push creating object onto thread local storage.
	object_stack_t* creatingObjectStack = static_cast< object_stack_t* >(g_creatingObjectStack.get());
	if (!creatingObjectStack)
	{
		creatingObjectStack = new object_stack_t();
		g_creatingObjectStack.set(creatingObjectStack);
	}

	creatingObjectStack->push_front(creatingObject);

	// Return pointer to uninitialized object memory.
	void* object = getObject(creatingObject);
	T_ASSERT_M ((size_t(object) & ~(align - 1)) == size_t(object), L"Object misaligned, allocator corrupt");

	return object;
}

void Heap::postConstructor(void* ptr)
{
	object_stack_t* creatingObjectStack = static_cast< object_stack_t* >(g_creatingObjectStack.get());
	T_ASSERT (creatingObjectStack);
	T_ASSERT (!creatingObjectStack->empty());

	Object* object = static_cast< Object* >(ptr);
	ObjectHeader* header = creatingObjectStack->front(); creatingObjectStack->pop_front();

	T_ASSERT (getObject(header) == object);
	T_ASSERT (header == getObjectHeader(object));

	Atomic::increment(g_stats.objects);
}

void Heap::registerRef(RefBase* ref, void* ptr)
{
	T_ASSERT (ref->m_prev == 0);
	T_ASSERT (ref->m_next == 0);
	T_ASSERT (ref->m_owner == 0);

	Atomic::increment(g_stats.references);

	incrementRef(ptr);

	object_stack_t* creatingObjectStack = static_cast< object_stack_t* >(g_creatingObjectStack.get());
	if (creatingObjectStack)
	{
		ObjectHeader* creatingObject = !creatingObjectStack->empty() ? creatingObjectStack->front() : 0;
		if (creatingObject)
		{
			uint8_t* objectTop = reinterpret_cast< uint8_t* >(getObject(creatingObject));
			uint8_t* objectEnd = objectTop + creatingObject->m_size;

			if ((uint8_t*)ref >= objectTop && (uint8_t*)ref < objectEnd)
			{
				ref->m_owner = creatingObject;
				creatingObject->m_crefs.push_front(ref);
				return;
			}
		}
	}

	{
		HEAP_LOCK;
		g_rootRefs.push_front(ref);
		Atomic::increment(g_stats.rootReferences);
	}
}

void Heap::unregisterRef(RefBase* ref, void* ptr)
{
	Atomic::decrement(g_stats.references);

	if (ref->m_owner)
	{
		ObjectHeader* owner = static_cast< ObjectHeader* >(ref->m_owner);
		T_ASSERT (owner->m_dead == 0);
		owner->m_crefs.remove(ref);
	}
	else
	{
		HEAP_LOCK;
		g_rootRefs.remove(ref);
		Atomic::decrement(g_stats.rootReferences);
	}

	decrementRef(ptr);

	--g_eventsUntilCollect;
}

void Heap::incrementRef(void* ptr)
{
	Object* object = reinterpret_cast< Object* >(ptr);
	ObjectHeader* header = getObjectHeader(object);
	if (header)
		Atomic::increment(header->m_refCount);
}

void Heap::decrementRef(void* ptr)
{
	if (ptr)
	{
		Object* object = reinterpret_cast< Object* >(ptr);
		ObjectHeader* header = getObjectHeader(object);
		if (header)
		{
			int32_t refCount = Atomic::decrement(header->m_refCount);
			if (refCount > 0)
			{
				if (!header->m_crefs.empty() && !header->m_pending)
				{
					HEAP_LOCK;
					if (!header->m_buffered)
					{
						header->m_buffered = 1;
						g_cycleObjects.push_back(header);
					}
				}
			}
			else if (refCount <= 0)
			{
				HEAP_LOCK;
				if (!header->m_crefs.empty())
				{
					if (header->m_buffered)
					{
						header->m_buffered = 0;
						g_cycleObjects.remove(header);
					}
				}
				freeObject(header);
			}
		}
	}
}

void Heap::exchangeRef(void** ptr1, void* ptr2)
{
	if (*ptr1 != ptr2)
	{
		Heap::incrementRef(ptr2);
		Heap::decrementRef(*ptr1);
		Atomic::exchange(*ptr1, ptr2);
	}
}

void Heap::lockRef()
{
	if (g_collectRunning)
	{
		Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
		do { currentThread->yield(); } while (g_collectRunning);
	}
	Atomic::increment(g_lockRefCount);
	T_ASSERT (!g_collectRunning);
}

void Heap::unlockRef()
{
	Atomic::decrement(g_lockRefCount);
}

namespace
{

	void Collect_visitRefs(IntrusiveList< RefBase >& refs, RefBase::IVisitor& visitor)
	{
		for (IntrusiveList< RefBase >::iterator i = refs.begin(); i != refs.end(); ++i)
			(*i)->visit(visitor);
	}

	template< int Condition >
	struct MarkVisitor : public RefBase::IVisitor
	{
		int32_t m_count;

		MarkVisitor()
		:	m_count(0)
		{
		}

		virtual void operator () (void* object)
		{
			ObjectHeader* header = getObjectHeader((Object*)object);
			if (header)
			{
				if (header->m_visited == Condition)
				{
					m_count++;
					header->m_visited = !Condition;
					Collect_visitRefs(header->m_crefs, *this);
				}
			}
		}
	};

	struct IncludeExternalVisitor : public RefBase::IVisitor
	{
		object_list_t& m_externalObjects;

		IncludeExternalVisitor(object_list_t& externalObjects)
		:	m_externalObjects(externalObjects)
		{
		}

		virtual void operator () (void* object)
		{
			ObjectHeader* header = getObjectHeader((Object*)object);
			if (header)
			{
				if (!header->m_pending && !header->m_visited)
				{
					header->m_pending = 1;
					m_externalObjects.push_back(header);
					Collect_visitRefs(header->m_crefs, *this);
				}
			}
		}
	};

}

void Heap::collect()
{
#if defined(T_HEAP_COLLECT_CYCLES)

	if (g_lockRefCount != 0)
		return;

	T_ASSERT (!g_collectRunning);
	g_collectRunning = true;
	T_ASSERT (g_lockRefCount == 0);

	log::debug << L"Heap cycle collector; begin" << Endl;

	object_list_t collectableObjects;
	object_list_t collectableExternalObjects;
	int32_t count1, count2;

	{
		HEAP_LOCK;

		if (!g_cycleObjects.empty())
		{
			// Mark reachable objects.
			{
				MarkVisitor< 0 > visitor;
				Collect_visitRefs(g_rootRefs, visitor);
				count1 = visitor.m_count;
			}

			log::debug << L"Heap cycle collector; " << count1 << L" reachable object(s) visited" << Endl;

			// Copy objects from cycle which hasn't been reached.
			while (!g_cycleObjects.empty())
			{
				ObjectHeader* header = g_cycleObjects.front(); g_cycleObjects.pop_front();
				T_ASSERT (header->m_dead == 0);
				T_ASSERT (header->m_buffered == 1);

				if (!header->m_visited)
				{
					header->m_pending = 1;
					collectableObjects.push_back(header);
				}

				header->m_buffered = 0;
			}

			log::debug << L"Heap cycle collector; " << collectableObjects.size() << L" candidates marked for collection" << Endl;

			// Ensure unreachable objects referenced from collectable objects also are in collectable list.
			for (object_list_t::iterator i = collectableObjects.begin(); i != collectableObjects.end(); ++i)
			{
				ObjectHeader* header = *i;
				T_ASSERT (header->m_dead == 0);
				T_ASSERT (header->m_buffered == 0);
				T_ASSERT (header->m_pending == 1);

				IncludeExternalVisitor visitor(collectableExternalObjects);
				Collect_visitRefs(header->m_crefs, visitor);
			}

			log::debug << L"Heap cycle collector; " << collectableExternalObjects.size() << L" external marked for collection" << Endl;

			// Reset reachable objects.
			{
				MarkVisitor< 1 > visitor;
				Collect_visitRefs(g_rootRefs, visitor);
				count2 = visitor.m_count;
			}

			log::debug << L"Heap cycle collector; " << count1 << L" reachable object(s) restored" << Endl;

			T_ASSERT (count1 == count2);
		}
	}

	g_collectRunning = false;

	//
	// Destruction phase; threads are allowed to continue from
	// here on as collectable objects should not be influencing
	// external objects.
	//

	// Append external objects to collectable list.
	collectableObjects.insert(
		collectableObjects.end(),
		collectableExternalObjects.begin(),
		collectableExternalObjects.end()
	);

	log::debug << L"Heap cycle collector; breaking internal references" << Endl;

	// Break references between collectable objects.
	for (object_list_t::iterator i = collectableObjects.begin(); i != collectableObjects.end(); ++i)
	{
		ObjectHeader* header = *i;
		T_ASSERT (header->m_dead == 0);
		T_ASSERT (header->m_buffered == 0);
		T_ASSERT (header->m_pending == 1);

		for (IntrusiveList< RefBase >::iterator j = header->m_crefs.begin(); j != header->m_crefs.end(); ++j)
		{
			for (object_list_t::iterator k = collectableObjects.begin(); k != collectableObjects.end(); ++k)
				(*j)->invalidate(getObject(*k));
		}
	}

	log::debug << L"Heap cycle collector; freeing objects" << Endl;

	// Collect unreachable objects.
	for (object_list_t::iterator i = collectableObjects.begin(); i != collectableObjects.end(); ++i)
	{
		ObjectHeader* header = *i;
		T_ASSERT (header->m_dead == 0);
		T_ASSERT (header->m_buffered == 0);
		T_ASSERT (header->m_pending == 1);

		freeObject(header);
	}

	log::debug << L"Heap cycle collector; finished" << Endl;

	Atomic::increment(g_stats.collects);
	
#endif
}

void Heap::getStats(HeapStats& outStats)
{
	HEAP_LOCK;
	outStats = g_stats;
}

}
