#ifndef traktor_amalgam_PhysicsServer_H
#define traktor_amalgam_PhysicsServer_H

#include "Amalgam/Game/IPhysicsServer.h"

namespace traktor
{

class PropertyGroup;

	namespace amalgam
	{

class IEnvironment;

class PhysicsServer : public IPhysicsServer
{
	T_RTTI_CLASS;

public:
	bool create(const PropertyGroup* defaultSettings, const PropertyGroup* settings);

	void destroy();

	void createResourceFactories(IEnvironment* environment);

	void createEntityFactories(IEnvironment* environment);

	int32_t reconfigure(const PropertyGroup* settings);

	void update(float simulationDeltaTime);

	virtual physics::PhysicsManager* getPhysicsManager() T_OVERRIDE T_FINAL;

private:
	Ref< physics::PhysicsManager > m_physicsManager;
};

	}
}

#endif	// traktor_amalgam_PhysicsServer_H
