/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/Game/IEnvironment.h"
#include "Amalgam/Game/Impl/PhysicsServer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Timer/Profiler.h"
#include "Physics/PhysicsFactory.h"
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

bool PhysicsServer::create(const PropertyGroup* defaultSettings, const PropertyGroup* settings)
{
	std::wstring physicsType = defaultSettings->getProperty< std::wstring >(L"Physics.Type");

	Ref< physics::PhysicsManager > physicsManager = dynamic_type_cast< physics::PhysicsManager* >(TypeInfo::createInstance(physicsType));
	if (!physicsManager)
		return false;

	physics::PhysicsCreateDesc pcd;
	pcd.timeScale = defaultSettings->getProperty< float >(L"Physics.TimeScale", 1.0f) * c_timeScale;
	pcd.solverIterations = defaultSettings->getProperty< int32_t >(L"Physics.SolverIterations", 10);

	if (!physicsManager->create(pcd))
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
	resourceManager->addFactory(new physics::PhysicsFactory());
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

void PhysicsServer::update(float simulationDeltaTime)
{
	T_PROFILER_SCOPE(L"PhysicsServer update");
	m_physicsManager->update(simulationDeltaTime, true);
}

physics::PhysicsManager* PhysicsServer::getPhysicsManager()
{
	return m_physicsManager;
}

	}
}
