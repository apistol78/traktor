#pragma once

#include "Runtime/IPhysicsServer.h"

namespace traktor
{

class PropertyGroup;

	namespace runtime
	{

class IEnvironment;

/*! \brief
 * \ingroup Runtime
 */
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

	virtual physics::PhysicsManager* getPhysicsManager() override final;

private:
	Ref< physics::PhysicsManager > m_physicsManager;
};

	}
}

