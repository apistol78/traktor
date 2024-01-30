/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ai/NavMeshFactory.h"
#include "Runtime/IEnvironment.h"
#include "Runtime/Impl/ResourceServer.h"
#include "Animation/AnimationResourceFactory.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Spark/MovieResourceFactory.h"
#include "Heightfield/HeightfieldFactory.h"
#include "Mesh/MeshResourceFactory.h"
#include "Spray/EffectFactory.h"
#include "Video/VideoFactory.h"
#include "Weather/Clouds/CloudMaskFactory.h"
#include "Resource/ResourceManager.h"

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
	render::IRenderSystem* renderSystem = environment->getRender()->getRenderSystem();

	m_resourceManager->addFactory(new ai::NavMeshFactory());
	m_resourceManager->addFactory(new animation::AnimationResourceFactory());
	m_resourceManager->addFactory(new mesh::MeshResourceFactory(renderSystem));
	m_resourceManager->addFactory(new hf::HeightfieldFactory());
	m_resourceManager->addFactory(new video::VideoFactory(renderSystem));
	m_resourceManager->addFactory(new weather::CloudMaskFactory());

	if (environment->getWorld())
	{
		const world::IEntityFactory* entityFactory = environment->getWorld()->getEntityFactory();
		m_resourceManager->addFactory(new spray::EffectFactory(entityFactory));
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
