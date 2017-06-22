/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_net_Network_H
#define traktor_net_Network_H

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

/*! \brief Network manager.
 * \ingroup Net
 */
class T_DLLCLASS Network
{
public:
	/*! \brief Initialize network.
	 * \return True if network is initialized and ready to be used.
	 */
	static bool initialize();

	/*! \brief Finalize network. */
	static void finalize();

private:
	static int32_t ms_initialized;
};

	}
}

#endif	// traktor_net_Network_H
