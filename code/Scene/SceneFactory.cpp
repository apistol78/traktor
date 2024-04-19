/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/ObjectStore.h"
#include "Database/Instance.h"
#include "Scene/Scene.h"
#include "Scene/SceneFactory.h"
#include "Scene/SceneResource.h"
#include "World/IEntityFactory.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.SceneFactory", SceneFactory, resource::IResourceFactory)

SceneFactory::SceneFactory(const world::IEntityFactory* entityFactory)
:	m_entityFactory(entityFactory)
{
}

bool SceneFactory::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet SceneFactory::getResourceTypes() const
{
	return makeTypeInfoSet< SceneResource >();
}

const TypeInfoSet SceneFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< Scene >();
}

bool SceneFactory::isCacheable(const TypeInfo& productType) const
{
	return false;
}

Ref< Object > SceneFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< const SceneResource > sceneResource = instance->getObject< SceneResource >();
	if (sceneResource)
		return sceneResource->createScene(m_entityFactory);
	else
		return nullptr;
}

}
