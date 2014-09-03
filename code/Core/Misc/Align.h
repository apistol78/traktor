#ifndef traktor_Align_H
#define traktor_Align_H

#include <cstddef>

namespace traktor
{

/*! \brief Align down value.
 * \ingroup Core
 */
template < typename T >
T alignDown(T v, size_t a)
{
	return (T)((size_t)v & ~(a - 1));
}

/*! \brief Align up value.
 * \ingroup Core.
 */
template < typename T >
T alignUp(T v, size_t a)
{
	return (T)(((size_t)v + a - 1) - ((size_t)v + a - 1) % a);
}

}

#endif	// traktor_Align_H
