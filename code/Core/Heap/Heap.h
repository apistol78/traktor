#ifndef traktor_Heap_H
#define traktor_Heap_H

#include <vector>
#include <list>
#include "Core/Containers/IntrusiveList.h"
#include "Core/Singleton/Singleton.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Event.h"
#include "Core/Timer/Timer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

class Thread;
class ThreadLocal;
class Allocator;
class Object;
class RefBase;
class Type;
class OutputStream;
class Functor;

struct ObjectInfo;

/*! \brief Managed object heap.
 * \ingroup Core
 *
 * The Heap is responsible of allocating and freeing managed object.
 * A managed object is an object allocated through the gc_new<> function
 * and kept by the Ref<> containers.
 * The Heap will automatically reclaim memory for an object when it
 * detects that there are no references to that specific object alive.
 * But contrary to more conventional reference counted schemes the Heap
 * implements a mark'n'sweep based collector scheme which is more efficient
 * but has the down-side of being non-deterministic, ie. you cannot rely
 * on the destructor being called as soon as the last reference is removed.
 *
 * \note
 * In order for an object to be collectable must either be seen
 * by a reference container or alive for a certain time.
 * This time is measured since it was allocated.
 */
class T_DLLCLASS Heap : public Singleton
{
	T_RTTI_CLASS(Heap)

public:
	static Heap& getInstance();

	/*! \brief Enter construction of new object.
	 *
	 * \param size Size of object in bytes.
	 * \param align Alignment requirement.
	 * \return Allocated memory for object.
	 */
	void* enterNewObject(size_t size, size_t align);

	/*! \brief Leave construction of object.
	 *
	 * \param ptr Object memory pointer returned by enterNewObject.
	 */
	void leaveNewObject(void* ptr);

	/*! \brief Add reference container to heap.
	 *
	 * \param ref Reference container.
	 */
	void addRef(RefBase* ref);

	/*! \brief Remove reference container from heap.
	 *
	 * \param ref Reference container.
	 */
	void removeRef(RefBase* ref);

	/*! \brief Invalidate all reference containers which reference object.
	 *
	 * \param obj Object
	 */
	void invalidateRefs(Object* obj);

	/*! \brief Invalidate all reference containers.
	 *
	 * \note
	 * All reference containers are invalidated and should
	 * therefor be used with extreme care.
	 */
	void invalidateAllRefs();

	/*! \brief Collect unused memory.
	 *
	 * \param wait Wait until collection is completed until return.
	 */
	void collect(bool wait = false);

	/*! \brief Collect all memory.
	 *
	 * \note
	 * Every managed object will be collected, alive or not,
	 * and therefor this method should be used with extreme care.
	 */
	void collectAll();

	/*! \brief Collect all memory allocated by a certain type.
	 *
	 * \note
	 * Every managed object will be collected, alive or not,
	 * and therefor this method should be used with extreme care.
	 */
	void collectAllOf(const Type& type);

	/*! \brief Get number of allocated objects.
	 *
	 * \return Number of allocated objects, alive or dead.
	 */
	int32_t getObjectCount() const;

	/*! \brief Get number of registered reference containers.
	 *
	 * \return Number of reference containers.
	 */
	int32_t getReferenceCount() const;

	/*! \brief Get number of reference containers referencing a given object.
	 *
	 * \note
	 * This method is extremely slow and should only be used
	 * as a debugging tool.
	 *
	 * \return Number of reference containers.
	 */
	uint32_t getReferenceCount(const Object* obj);

	/*! \brief Dump information about allocated objects.
	 *
	 * \param os Target output stream.
	 */
	void dump(OutputStream& os) const;

protected:
	void create();

	virtual void destroy();

private:
	mutable Semaphore m_lock;
	Allocator* m_allocator;
	ThreadLocal* m_creatingObjectStack;
	std::vector< IntrusiveList< ObjectInfo >* > m_creatingObjectStacks;
	IntrusiveList< ObjectInfo > m_objects;
	IntrusiveList< RefBase > m_rootRefs;
	Semaphore m_destructQueueLock;
	Event m_destructQueueEvent;
	std::list< Functor* > m_destructQueue;
	Thread* m_destructThread;
	int32_t m_objectsSinceGC;
	int32_t m_refsRemovedSinceGC;
	int32_t m_objectCount;
	int32_t m_referenceCount;
	Timer m_timer;

	Heap();

	~Heap();

	void destruct(IntrusiveList< ObjectInfo > collectables);

	void destructThread();
};

}

#endif	// traktor_Heap_H
