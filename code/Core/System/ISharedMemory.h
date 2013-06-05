#ifndef traktor_ISharedMemory_H
#define traktor_ISharedMemory_H

#include "Core/Ref.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

/*! \brief Process shared memory.
*
* Simple wrapper for shared memory across several processes.
* Designed around a simple "single writer/multiple readers" pattern.
*/
class T_DLLCLASS ISharedMemory : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Read data from shared memory.
	 *
	 * \param exclusive If exclusive access to memory is required.
	 * \return Stream to shared memory, null if failed to acquire access.
	 */
	virtual Ref< IStream > read(bool exclusive = false) = 0;

	/*! \brief Write data to shared memory.
	 *
	 * \return Stream to shared memory, null if failed to acquire access.
	 */
	virtual Ref< IStream > write() = 0;

	/*! \brief Clear shared memory.
	 */
	virtual bool clear() = 0;
};

}

#endif	// traktor_ISharedMemory_H
