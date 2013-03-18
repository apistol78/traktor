#ifndef traktor_FbxLock_H
#define traktor_FbxLock_H

#include "Core/Thread/Semaphore.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Global lock to ensure serialization of
 *  FBX usage.
 * \ingroup Core
 */
extern Semaphore T_DLLCLASS g_fbxLock;

}

#endif	// traktor_FbxLock_H
