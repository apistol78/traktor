#ifndef traktor_Heap_H
#define traktor_Heap_H

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

class RefBase;
struct HeapStats;

/*! \brief Managed object heap.
 * \ingroup Core
 */
class T_DLLCLASS Heap
{
public:
	/*! \brief Enter construction of new object.
	 *
	 * \param size Size of object in bytes.
	 * \param align Alignment requirement.
	 * \return Allocated memory for object.
	 */
	static void* preConstructor(size_t size, size_t align);

	/*! \brief Leave construction of object.
	 *
	 * \param ptr Object memory pointer returned by enterNewObject.
	 */
	static void postConstructor(void* ptr);

	/*! \brief Add reference container to heap.
	 *
	 * \param ref Reference container.
	 * \param ptr Initial object; ref count will be incremented.
	 */
	static void registerRef(RefBase* ref, void* ptr);

	/*! \brief Remove reference container from heap.
	 *
	 * \param ref Reference container.
	 * \param ptr Existing object; ref count will be decremented.
	 */
	static void unregisterRef(RefBase* ref, void* ptr);

	/*! \brief Increment reference count on object.
	 *
	 * \param ptr Object pointer.
	 */
	static void incrementRef(void* ptr);

	/*! \brief Decrement reference count on object.
	 *
	 * \param ptr Object pointer.
	 */
	static void decrementRef(void* ptr);

	/*! \brief Exchange reference.
	 *
	 * \param ptr1 Current reference pointer.
	 * \param ptr2 New reference pointer.
	 */
	static void exchangeRef(void** ptr1, void* ptr2);

	/*! \brief Lock reference. */
	static void lockRef();

	/*! \brief Unlock reference. */
	static void unlockRef();

	/*! \brief Collect unused memory. */
	static void collect();

	/*! \brief Get heap statistics.
	 *
	 * \param outStats Heap statistics.
	 */
	static void getStats(HeapStats& outStats);
};

}

#endif	// traktor_Heap_H
