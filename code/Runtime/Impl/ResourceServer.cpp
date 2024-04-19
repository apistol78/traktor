/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/ObjectStore.h"
#include "Runtime/IEnvironment.h"
#include "Runtime/Impl/ResourceServer.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Render/IRenderSystem.h"
#include "Resource/IResourceFactory.h"
#include "Resource/ResourceManager.h"
#include "World/IEntityFactory.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.ResourceServer", ResourceServer, IResourceServer)

bool ResourceServer::create(const PropertyGroup* settings, db::Database* database)
{
	m_resourceManager = new resource::ResourceManager(database, settings->getProperty< bool >(L"Resource.Verbose", false));
	return true;
}

void ResourceServer::destroy()
{
	safeDestroy(m_resourceManager);
}

void ResourceServer::createResourceFactories(IEnvironment* environment)
{
	// Setup object store with relevant systems.
	ObjectStore objectStore;
	objectStore.set(environment->getRender()->getRenderSystem());
	objectStore.set(environment->getWorld()->getEntityFactory());

	// Create instances of all resource factories.
	const TypeInfoSet resourceFactoryTypes = type_of< resource::IResourceFactory >().findAllOf(false);
	for (const auto& resourceFactoryType : resourceFactoryTypes)
	{
		if (!resourceFactoryType->isInstantiable())
			continue;

		Ref< resource::IResourceFactory > resourceFactory = dynamic_type_cast< resource::IResourceFactory* >(resourceFactoryType->createInstance());
		if (!resourceFactory)
			continue;

		if (!resourceFactory->initialize(objectStore))
			continue;

		m_resourceManager->addFactory(resourceFactory);
		T_DEBUG(L"Resource factory \"" << type_name(resourceFactory) << L"\" initialized.");
	}
}

int32_t ResourceServer::reconfigure(const PropertyGroup* settings)
{
	return CrUnaffected;
}

void ResourceServer::performCleanup()
{
	m_resourceManager->unloadUnusedResident();
}

resource::IResourceManager* ResourceServer::getResourceManager()
{
	return m_resourceManager;
}

}
