#ifndef traktor_amalgam_PhysicsServer_H
#define traktor_amalgam_PhysicsServer_H

#include "Amalgam/IPhysicsServer.h"

namespace traktor
{

class Settings;

	namespace amalgam
	{

class IEnvironment;

class PhysicsServer : public IPhysicsServer
{
	T_RTTI_CLASS;

public:
	bool create(const Settings* settings, float simulationDeltaTime);

	void destroy();

	void createResourceFactories(IEnvironment* environment);

	void createEntityFactories(IEnvironment* environment);

	int32_t reconfigure(const Settings* settings);

	void update();

	virtual physics::PhysicsManager* getPhysicsManager();

private:
	Ref< physics::PhysicsManager > m_physicsManager;
};

	}
}

#endif	// traktor_amalgam_PhysicsServer_H
