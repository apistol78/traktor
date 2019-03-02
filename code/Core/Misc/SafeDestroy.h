#pragma once

namespace traktor
{

/*! \brief Safe destroy call.
 *
 * Helper function to reduce code bloat
 * in common pattern.
 */
template < typename T >
void safeDestroy(T& tv)
{
	if (tv)
	{
		T ttv = tv; tv = 0;
		ttv->destroy();
	}
}

/*! \brief Safe close call.
 *
 * Helper function to reduce code bloat
 * in common pattern.
 */
template < typename T >
void safeClose(T& tv)
{
	if (tv)
	{
		T ttv = tv; tv = 0;
		ttv->close();
	}
}

}

