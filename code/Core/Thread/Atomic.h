#ifndef traktor_Atomic_H
#define traktor_Atomic_H

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Thread atomic operations.
 * \ingroup Core
 *
 * Perform atomic operations on variables.
 */
struct T_DLLCLASS Atomic
{
	/*! \brief Increment variable.
	 *
	 * \return Result value of variable.
	 */
	static int32_t increment(int32_t& value);

	/*! \brief Decrement variable.
	 *
	 * \return Result value of variable.
	 */
	static int32_t decrement(int32_t& value);

	/*! \brief Set value of variable. */
	static uint32_t exchange(uint32_t& s, uint32_t v);

	/*! \brief Set value of variable. */
	static uint64_t exchange(uint64_t& s, uint64_t v);

	/*! \brief Set value of variable. */
	template < typename T >
	static T exchange(T& s, T v)
	{
		if (sizeof(T) <= sizeof(uint32_t))
			return (T)exchange(*(uint32_t*)&s, *(uint32_t*)&v);
		else
			return (T)exchange(*(uint64_t*)&s, *(uint64_t*)&v);
	}
};

}

#endif	// traktor_Atomic_H
