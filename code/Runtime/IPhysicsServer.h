#pragma once

#include "Runtime/IServer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
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

	namespace runtime
	{

/*! Physics server.
 * \ingroup Runtime
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

