#include "Amalgam/IEnvironment.h"
#include "Amalgam/Impl/LibraryHelper.h"
#include "Amalgam/Impl/PhysicsServer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Physics/MeshFactory.h"
#include "Physics/PhysicsManager.h"
#include "Physics/World/EntityFactory.h"
#include "Resource/IResourceManager.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

const float c_timeScale = 1.25f;	//< Make simulation 25% faster than normal; empirically measured to make simulation feel more natural.

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.PhysicsServer", PhysicsServer, IPhysicsServer)

bool PhysicsServer::create(const PropertyGroup* defaultSettings, const PropertyGroup* settings, float simulationDeltaTime)
{
	std::wstring physicsType = defaultSettings->getProperty< PropertyString >(L"Physics.Type");
	float timeScale = settings->getProperty< PropertyFloat >(L"Physics.TimeScale", 1.0f);

	Ref< physics::PhysicsManager > physicsManager = loadAndInstantiate< physics::PhysicsManager >(physicsType);
	if (!physicsManager)
		return false;

	if (!physicsManager->create(simulationDeltaTime, timeScale * c_timeScale))
	{
		log::error << L"Physics server failed; unable to create physics manager" << Endl;
		return false;
	}

	m_physicsManager = physicsManager;
	return true;
}

void PhysicsServer::destroy()
{
	safeDestroy(m_physicsManager);
}

void PhysicsServer::createResourceFactories(IEnvironment* environment)
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	resourceManager->addFactory(new physics::MeshFactory(environment->getDatabase()));
}

void PhysicsServer::createEntityFactories(IEnvironment* environment)
{
	world::IEntityEventManager* eventManger = environment->getWorld()->getEntityEventManager();
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	environment->getWorld()->addEntityFactory(new physics::EntityFactory(eventManger, resourceManager, m_physicsManager));
}

int32_t PhysicsServer::reconfigure(const PropertyGroup* settings)
{
	return CrUnaffected;
}

void PhysicsServer::update()
{
	m_physicsManager->update(true);
}

physics::PhysicsManager* PhysicsServer::getPhysicsManager()
{
	return m_physicsManager;
}

	}
}
