#ifndef traktor_amalgam_IServer_H
#define traktor_amalgam_IServer_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

/*! \brief System server interface.
 * \ingroup Amalgam
 *
 * System servers are system pieces such
 * as rendering, input, audio etc.
 * Servers are setup using configuration values
 * from settings and must be able to be reconfigured
 * during runtime.
 */
class T_DLLCLASS IServer : public Object
{
	T_RTTI_CLASS;
};

	}
}

#endif	// traktor_amalgam_IServer_H
