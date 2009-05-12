#ifndef traktor_SharedMemory_H
#define traktor_SharedMemory_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Stream;

/*! \brief Process shared memory.
*
* Simple wrapper for shared memory across several processes.
* Designed around a simple "single writer/multiple readers" pattern.
*/
class T_DLLCLASS SharedMemory : public Object
{
	T_RTTI_CLASS(SharedMemory)

public:
	/*! \brief Read data from shared memory.
	 *
	 * \param exclusive If exclusive access to memory is required.
	 * \return Stream to shared memory, null if failed to acquire access.
	 */
	virtual Stream* read(bool exclusive = false) = 0;

	/*! \brief Write data to shared memory.
	 *
	 * \return Stream to shared memory, null if failed to acquire access.
	 */
	virtual Stream* write() = 0;
};

}

#endif	// traktor_SharedMemory_H
