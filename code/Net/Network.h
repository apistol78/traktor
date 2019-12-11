#pragma once

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

/*! Network manager.
 * \ingroup Net
 */
class T_DLLCLASS Network
{
public:
	/*! Initialize network.
	 * \return True if network is initialized and ready to be used.
	 */
	static bool initialize();

	/*! Finalize network. */
	static void finalize();

private:
	static int32_t ms_initialized;
};

	}
}

