#ifndef traktor_amalgam_IPhysicsServer_H
#define traktor_amalgam_IPhysicsServer_H

#include "Amalgam/Game/IServer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class PhysicsManager;

	}

	namespace amalgam
	{

/*! \brief Physics server.
 * \ingroup Amalgam
 *
 * "Physics.Type"		- Physics manager type.
 */
class T_DLLCLASS IPhysicsServer : public IServer
{
	T_RTTI_CLASS;

public:
	virtual physics::PhysicsManager* getPhysicsManager() = 0;
};

	}
}

#endif	// traktor_amalgam_IPhysicsServer_H
