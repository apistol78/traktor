#include <algorithm>
#include <map>
#include "Core/Heap/HeapConfig.h"
#include "Core/Heap/Heap.h"
#include "Core/Heap/StdAllocator.h"
#if defined(T_HEAP_FAST_ALLOCATOR)
#	include "Core/Heap/FastAllocator.h"
#endif
#if defined(T_HEAP_DEBUG_ALLOCATOR)
#	include "Core/Heap/DebugAllocator.h"
#endif
#include "Core/Heap/Ref.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/ThreadLocal.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/Atomic.h"
#include "Core/Log/Log.h"

namespace traktor
{

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
struct ObjectInfo
{
	struct 
	{
		unsigned m_sizeOrTick : 29;	//!< Size of object during object creation. Clock tick after object was created.
		unsigned m_keep : 1;		//!< Keep object flag; used by mark phase to mark objects to be kept.
		unsigned m_visited : 1;		//!< Visited flag; used when traversing references to indicate if object has been already visited.
		unsigned m_referenced : 1;	//!< Referenced flag; i.e. it's been visible through a reference in mark phase.
	};
	IntrusiveList< RefBase > m_crefs;
	ObjectInfo* m_prev;
	ObjectInfo* m_next;
#if defined(_WIN64) || defined(_XBOX) || defined(_PS3)
	uint8_t m_padding[4];
#endif

	ObjectInfo()
	:	m_sizeOrTick(0)
	,	m_keep(0)
	,	m_visited(0)
	,	m_referenced(0)
	,	m_prev(0)
	,	m_next(0)
	{
	}
};

#pragma pack()

	namespace
	{

#if defined(T_HEAP_THREAD_SAFE)
#	define HEAP_LOCK Acquire< Semaphore > __prvlock__(m_lock);
#else
#	define HEAP_LOCK
#endif

const int32_t c_objectsThreshold = 1000;			//!< Number of objects allocated before issuing GC.
const int32_t c_refsRemovedThreshold = 30000;		//!< Number of references removed before issuing GC.
const int32_t c_promoteNonVisitedThreshold = 100;	//!< Milliseconds until objects get promoted from non-visited into collectable.
const int32_t c_maxDestructQueueSize = 64;			//!< Maximum number of destruction queues; collector will force a block if this is exceeded.

T_FORCE_INLINE size_t getAllocSize(size_t objectSize)
{
	return objectSize + sizeof(ObjectInfo);
}

T_FORCE_INLINE Object* getObject(ObjectInfo* info)
{
	return reinterpret_cast< Object* >(reinterpret_cast< uint8_t* >(info) + sizeof(ObjectInfo));
}

T_FORCE_INLINE ObjectInfo* getInfo(Object* object)
{
	return reinterpret_cast< ObjectInfo* >(reinterpret_cast< uint8_t* >(object) - sizeof(ObjectInfo));
}

T_FORCE_INLINE const Object* getObject(const ObjectInfo* info)
{
	return reinterpret_cast< const Object* >(reinterpret_cast< const uint8_t* >(info) + sizeof(ObjectInfo));
}

T_FORCE_INLINE const ObjectInfo* getInfo(const Object* object)
{
	return reinterpret_cast< const ObjectInfo* >(reinterpret_cast< const uint8_t* >(object) - sizeof(ObjectInfo));
}

T_FORCE_INLINE void visitRefs(IntrusiveList< RefBase >& refs, RefBase::Visitor& visitor)
{
	for (IntrusiveList< RefBase >::iterator i = refs.begin(); i != refs.end(); ++i)
		i->visit(visitor);
}

struct MarkVisitor : public RefBase::Visitor
{
	virtual void operator () (Object* object)
	{
		if (!object->isManaged())
			return;

		ObjectInfo* info = getInfo(object);
		if (info->m_visited)
			return;

		info->m_keep = 1;
		info->m_visited = 1;
		info->m_referenced = 1;

		visitRefs(info->m_crefs, *this);
	}
};

struct CountRefVisitor : public RefBase::Visitor
{
	const Object* m_object;
	uint32_t m_count;

	CountRefVisitor(const Object* object)
	:	m_object(object)
	,	m_count(0)
	{
	}

	virtual void operator () (Object* object)
	{
		if (object == m_object)
			++m_count;

		if (!object->isManaged())
			return;

		ObjectInfo* info = getInfo(object);
		if (!info->m_visited)
		{
			info->m_visited = 1;
			visitRefs(info->m_crefs, *this);
		}
	}
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Heap", Heap, Singleton)

Heap::Heap()
:	m_allocator(0)
,	m_creatingObjectStack(new ThreadLocal())
,	m_destructThread(0)
,	m_objectsSinceGC(0)
,	m_refsRemovedSinceGC(0)
,	m_objectCount(0)
,	m_referenceCount(0)
{
	m_timer.start();
}

Heap::~Heap()
{
	T_EXCEPTION_GUARD_BEGIN

	collectAll();

#if defined(T_HEAP_CONCURRENT_COLLECT)
	if (m_destructThread)
	{
		m_destructThread->stop();
		ThreadManager::getInstance().destroy(m_destructThread);
		m_destructThread = 0;
	}
#endif

	delete m_allocator;
	delete m_creatingObjectStack;

	for (std::vector< IntrusiveList< ObjectInfo >* >::iterator i = m_creatingObjectStacks.begin(); i != m_creatingObjectStacks.end(); ++i)
		delete *i;

	m_creatingObjectStacks.resize(0);

	T_EXCEPTION_GUARD_END
}

Heap& Heap::getInstance()
{
	static Heap* s_instance = 0;
	if (!s_instance)
	{
		s_instance = new Heap();
		SingletonManager::getInstance().add(s_instance);

		// Finally create the Heap; we need to do this after we've added
		// the instance to the singleton manager as create will use
		// other singletons.
		s_instance->create();
	}
	return *s_instance;
}

void* Heap::enterNewObject(size_t size, size_t align)
{
	T_ASSERT_M (size > 0, L"Invalid size");
	T_ASSERT_M (align <= 16, L"Alignment must be 16 or less");

	if (m_objectsSinceGC >= c_objectsThreshold && m_refsRemovedSinceGC >= c_refsRemovedThreshold)
	{
		m_objectsSinceGC = 0;
		m_refsRemovedSinceGC = 0;
		collect();
	}

	// Allocate object, including object information.
	void* ptr = m_allocator->alloc(getAllocSize(size), align);
	if (!ptr)
		T_FATAL_ERROR;

	ObjectInfo* creatingObject = new (ptr) ObjectInfo();
	creatingObject->m_sizeOrTick = size;

	// Push creating object onto thread local storage.
	{
		HEAP_LOCK

		IntrusiveList< ObjectInfo >* creatingObjectStack = static_cast< IntrusiveList< ObjectInfo >* >(m_creatingObjectStack->get());
		if (!creatingObjectStack)
		{
			creatingObjectStack = new IntrusiveList< ObjectInfo >();
			m_creatingObjectStack->set(creatingObjectStack);
			m_creatingObjectStacks.push_back(creatingObjectStack);
		}
		creatingObjectStack->push_front(creatingObject);
	}

	// Return pointer to uninitialized object memory.
	void* object = getObject(creatingObject);
	T_ASSERT_M ((size_t(object) & ~(align - 1)) == size_t(object), L"Object misaligned, allocator corrupt");

	return object;
}

void Heap::leaveNewObject(void* ptr)
{
	HEAP_LOCK

	IntrusiveList< ObjectInfo >* creatingObjectStack = static_cast< IntrusiveList< ObjectInfo >* >(m_creatingObjectStack->get());
	T_ASSERT (creatingObjectStack);
	T_ASSERT (!creatingObjectStack->empty());

	ObjectInfo* info = creatingObjectStack->front();
	creatingObjectStack->pop_front();

	T_ASSERT (info == getInfo(static_cast< Object* >(ptr)));
	getObject(info)->m_managed = true;

	info->m_sizeOrTick = uint32_t(m_timer.getElapsedTime() * 1000.0f);

	m_objects.push_front(info);

	Atomic::increment(m_objectsSinceGC);
	Atomic::increment(m_objectCount);
}

void Heap::addRef(RefBase* ref)
{
	T_ASSERT (ref->m_prev == 0);
	T_ASSERT (ref->m_next == 0);
	T_ASSERT (ref->m_owner == 0);

	Atomic::increment(m_referenceCount);

#if defined(T_DEBUG_REFERENCE_ADDRESS)

	// Get instantiation address from call stack.
	uint32_t adress = 0UL;
	__asm
	{
		mov eax, [ebp]
		mov eax, [eax + 4]
		mov adress, eax
	}
	ref->m_adress = adress;

#endif

	// Get object being created, thus register reference as member reference.
	IntrusiveList< ObjectInfo >* creatingObjectStack = static_cast< IntrusiveList< ObjectInfo >* >(m_creatingObjectStack->get());
	if (creatingObjectStack)
	{
		ObjectInfo* creatingObject = !creatingObjectStack->empty() ? creatingObjectStack->front() : 0;
		if (creatingObject)
		{
			uint8_t* objectTop = reinterpret_cast< uint8_t* >(getObject(creatingObject));
			uint8_t* objectEnd = objectTop + creatingObject->m_sizeOrTick;

			if ((uint8_t*)ref >= objectTop && (uint8_t*)ref < objectEnd)
			{
				ref->m_owner = getObject(creatingObject);
				creatingObject->m_crefs.push_front(ref);
				return;
			}
		}
	}

	// Add reference to root references list.
	{
		HEAP_LOCK
		m_rootRefs.push_front(ref);
	}
}

void Heap::removeRef(RefBase* ref)
{
	HEAP_LOCK

	T_ASSERT (m_referenceCount > 0);
	
	if (ref->m_owner)
	{
		ObjectInfo* owner = getInfo(ref->m_owner);
		owner->m_crefs.remove(ref);
	}
	else
	{
		m_rootRefs.remove(ref);
	}

	Atomic::increment(m_refsRemovedSinceGC);
	Atomic::decrement(m_referenceCount);
}

void Heap::invalidateRefs(Object* obj)
{
	HEAP_LOCK

	for (IntrusiveList< RefBase >::iterator i = m_rootRefs.begin(); i != m_rootRefs.end(); ++i)
		i->invalidate(obj);

	for (IntrusiveList< ObjectInfo >::iterator i = m_objects.begin(); i != m_objects.end(); ++i)
	{
		for (IntrusiveList< RefBase >::iterator j = i->m_crefs.begin(); j != i->m_crefs.end(); ++j)
			j->invalidate(obj);
	}

	for (std::vector< IntrusiveList< ObjectInfo >* >::iterator i = m_creatingObjectStacks.begin(); i != m_creatingObjectStacks.end(); ++i)
	{
		IntrusiveList< ObjectInfo >& objects = *(*i);
		for (IntrusiveList< ObjectInfo >::iterator j = objects.begin(); j != objects.end(); ++j)
		{
			for (IntrusiveList< RefBase >::iterator k = j->m_crefs.begin(); k != j->m_crefs.end(); ++k)
				k->invalidate(obj);
		}
	}
}

void Heap::invalidateAllRefs()
{
	HEAP_LOCK
	
	for (IntrusiveList< RefBase >::iterator i = m_rootRefs.begin(); i != m_rootRefs.end(); ++i)
		i->invalidate();

	for (IntrusiveList< ObjectInfo >::iterator i = m_objects.begin(); i != m_objects.end(); ++i)
	{
		for (IntrusiveList< RefBase >::iterator j = i->m_crefs.begin(); j != i->m_crefs.end(); ++j)
			j->invalidate();
	}

	for (std::vector< IntrusiveList< ObjectInfo >* >::iterator i = m_creatingObjectStacks.begin(); i != m_creatingObjectStacks.end(); ++i)
	{
		IntrusiveList< ObjectInfo >& objects = *(*i);
		for (IntrusiveList< ObjectInfo >::iterator j = objects.begin(); j != objects.end(); ++j)
		{
			for (IntrusiveList< RefBase >::iterator k = j->m_crefs.begin(); k != j->m_crefs.end(); ++k)
				k->invalidate();
		}
	}
}

void Heap::collect(bool wait)
{
	IntrusiveList< ObjectInfo > collectables;
	MarkVisitor visitor;

	{
		HEAP_LOCK

		uint32_t promotionTick = uint32_t(m_timer.getElapsedTime() * 1000.0f);

		// Mark reachable objects.
		for (std::vector< IntrusiveList< ObjectInfo >* >::iterator i = m_creatingObjectStacks.begin(); i != m_creatingObjectStacks.end(); ++i)
		{
			for (IntrusiveList< ObjectInfo >::iterator j = (*i)->begin(); j != (*i)->end(); ++j)
			{
				T_ASSERT (j->m_visited == 0);
				visitRefs(j->m_crefs, visitor);
			}
		}

		for (IntrusiveList< ObjectInfo >::iterator i = m_objects.begin(); i != m_objects.end(); ++i)
		{
			if (!i->m_referenced && (promotionTick - i->m_sizeOrTick) < c_promoteNonVisitedThreshold)
				visitRefs(i->m_crefs, visitor);
		}

		visitRefs(m_rootRefs, visitor);

		// Gather collectables.
		for (IntrusiveList< ObjectInfo >::iterator i = m_objects.begin(); !m_objects.empty() && i != m_objects.end(); )
		{
			i->m_visited = 0;

			if (i->m_keep)
			{
				i->m_keep = 0;
				i++;
			}
			else
			{
				ObjectInfo* collected = *i++;

				m_objects.remove(collected);
				collectables.push_front(collected);

				Atomic::decrement(m_objectCount);
			}
		}
	}

	// Destroy collectables.
	{
#if defined(T_HEAP_CONCURRENT_COLLECT)
		bool enqueued = false;
		if (!wait)
		{
			m_destructQueueLock.acquire();
			if (m_destructQueue.size() < c_maxDestructQueueSize)
			{
				Functor* task = makeFunctor(this, &Heap::destruct, collectables);
				if (task)
				{
					m_destructQueue.push_back(task);
					m_destructQueueEvent.pulse();
					enqueued = true;
				}
			}
			m_destructQueueLock.release();
		}
		if (!enqueued)
			destruct(collectables);
#else
		destruct(collectables);
#endif
	}
}

void Heap::collectAll()
{
	{
		HEAP_LOCK

		invalidateAllRefs();

		while (!m_objects.empty())
		{
			ObjectInfo* info = m_objects.front();
			m_objects.pop_front();

			getObject(info)->~Object();
			m_allocator->free(info);

			Atomic::decrement(m_objectCount);
		}

		T_ASSERT (!m_objectCount);
	}
}

void Heap::collectAllOf(const Type& type)
{
	{
		HEAP_LOCK

		for (IntrusiveList< ObjectInfo >::iterator i = m_objects.begin(); i != m_objects.end(); )
		{
			Object* object = getObject(*i);
			if (is_type_of(type, object->getType()))
			{
				ObjectInfo* info = *i;
				m_objects.erase(i++);

				invalidateRefs(object);

				object->~Object();
				m_allocator->free(info);

				Atomic::decrement(m_objectCount);
			}
			else
				++i;
		}
	}
}

int32_t Heap::getObjectCount() const
{
	return m_objectCount;
}

int32_t Heap::getReferenceCount() const
{
	return m_referenceCount;
}

uint32_t Heap::getReferenceCount(const Object* obj)
{
	HEAP_LOCK

	CountRefVisitor visitor(obj);

	for (std::vector< IntrusiveList< ObjectInfo >* >::iterator i = m_creatingObjectStacks.begin(); i != m_creatingObjectStacks.end(); ++i)
	{
		for (IntrusiveList< ObjectInfo >::iterator j = (*i)->begin(); j != (*i)->end(); ++j)
		{
			T_ASSERT (j->m_visited == 0);
			visitRefs(j->m_crefs, visitor);
		}
	}

	for (IntrusiveList< ObjectInfo >::iterator i = m_objects.begin(); i != m_objects.end(); ++i)
		visitRefs(i->m_crefs, visitor);

	visitRefs(m_rootRefs, visitor);

	for (IntrusiveList< ObjectInfo >::iterator i = m_objects.begin(); i != m_objects.end(); ++i)
		i->m_visited = 0;

	return visitor.m_count;
}

void Heap::dump(OutputStream& os) const
{
	HEAP_LOCK

	std::map< const Type*, uint32_t > objectCount;

	for (IntrusiveList< ObjectInfo >::const_iterator i = m_objects.begin(); i != m_objects.end(); ++i)
	{
		const Object* o = getObject(*i);
		objectCount[&o->getType()]++;
	}

	for (std::map< const Type*, uint32_t >::iterator i = objectCount.begin(); i != objectCount.end(); ++i)
		os << i->first->getName() << L" : \t" << i->second << Endl;
}

void Heap::create()
{
	T_ASSERT_M((sizeof(ObjectInfo) & 15) == 0, L"Incorrect size of ObjectInfo structure");

	m_allocator = new StdAllocator();

	/*lint -e423*/
#if defined(T_HEAP_FAST_ALLOCATOR)
	m_allocator = new FastAllocator(m_allocator);
#endif
#if defined(T_HEAP_DEBUG_ALLOCATOR)
	m_allocator = new DebugAllocator(m_allocator);
#endif
	/*lint -restore*/

#if defined(T_HEAP_CONCURRENT_COLLECT)
	m_destructThread = ThreadManager::getInstance().create(makeFunctor(this, &Heap::destructThread), L"Heap destruct thread");
	T_ASSERT (m_destructThread);

	m_destructThread->start();
#endif
}

void Heap::destroy()
{
	delete this;
}

void Heap::destruct(IntrusiveList< ObjectInfo > collectables)
{
	MarkVisitor visitor;

	while (!collectables.empty())
	{
		{
			HEAP_LOCK

			// Keep objects which are members to objects being collected.
			for (IntrusiveList< ObjectInfo >::iterator i = collectables.begin(); i != collectables.end(); ++i)
			{
				if (i->m_keep)
					continue;

				i->m_visited = 0;
				visitRefs(i->m_crefs, visitor);

				if (i->m_keep)
				{
					// Circular reference; need to invalidate all references to this object prior
					// to collection.
					for (IntrusiveList< ObjectInfo >::iterator j = collectables.begin(); j != collectables.end(); ++j)
					{
						for (IntrusiveList< RefBase >::iterator k = j->m_crefs.begin(); k != j->m_crefs.end(); ++k)
							k->invalidate(getObject(*i));
					}
				}
			}

			// Ensure flags on living objects are reset.
			for (IntrusiveList< ObjectInfo >::iterator i = m_objects.begin(); i != m_objects.end(); ++i)
			{
				i->m_visited = 0;
				i->m_keep = 0;
			}
		}

		// Collect objects which are still marked for collection.
		for (IntrusiveList< ObjectInfo >::iterator i = collectables.begin(); i != collectables.end(); )
		{
			i->m_visited = 0;

			if (i->m_keep)
			{
				i->m_keep = 0;
				i++;
			}
			else
			{
				ObjectInfo* info = *i++;
				collectables.remove(info);

				getObject(info)->~Object();
				m_allocator->free(info);
			}
		}
	}
}

void Heap::destructThread()
{
	T_ASSERT (m_destructThread);
	while (!m_destructThread->stopped())
	{
		if (!m_destructQueueEvent.wait(100))
			continue;

		m_destructQueueLock.acquire();
		T_ASSERT (!m_destructQueue.empty());
		Functor* task = m_destructQueue.front();
		m_destructQueue.pop_front();
		T_ASSERT (task);
		m_destructQueueLock.release();

		(*task)();

		m_destructQueueLock.acquire();
		delete task;
		m_destructQueueLock.release();
	}
}

}
