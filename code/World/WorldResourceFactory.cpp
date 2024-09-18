/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Log/Log.h"
#include "Database/Instance.h"
#include "World/EntityData.h"
#include "World/IEntityEvent.h"
#include "World/IEntityEventData.h"
#include "World/IEntityFactory.h"
#include "World/IrradianceGrid.h"
#include "World/IrradianceGridResource.h"
#include "World/WorldResourceFactory.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldResourceFactory", WorldResourceFactory, resource::IResourceFactory)

WorldResourceFactory::WorldResourceFactory(render::IRenderSystem* renderSystem, const IEntityFactory* entityFactory)
:	m_renderSystem(renderSystem)
,	m_entityFactory(entityFactory)
{
}

bool WorldResourceFactory::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet WorldResourceFactory::getResourceTypes() const
{
	TypeInfoSet typeInfoSet;
	typeInfoSet.insert< EntityData >();
	typeInfoSet.insert< IEntityEventData >();
	typeInfoSet.insert< IrradianceGridResource >();
	return typeInfoSet;
}

const TypeInfoSet WorldResourceFactory::getProductTypes(const TypeInfo& resourceType) const
{
	if (is_type_a< EntityData >(resourceType))
		return makeTypeInfoSet< EntityData >();
	else if (is_type_a< IEntityEventData >(resourceType))
		return makeTypeInfoSet< IEntityEvent >();
	else if (is_type_a< IrradianceGridResource >(resourceType))
		return makeTypeInfoSet< IrradianceGrid >();
	else
		return TypeInfoSet();
}

bool WorldResourceFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > WorldResourceFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	if (is_type_a< EntityData >(productType))
	{
		return instance->getObject< EntityData >();
	}
	else if (is_type_a< IEntityEvent >(productType))
	{
		if (!m_entityFactory)
			return nullptr;

		Ref< const IEntityEventData > eventData = instance->getObject< IEntityEventData >();
		if (!eventData)
			return nullptr;

		Ref< IEntityEvent > event = m_entityFactory->createEntityEvent(nullptr, *eventData);
		if (!event)
			return nullptr;

		return event;
	}
	else if (is_type_a< IrradianceGrid >(productType))
	{
		if (!m_renderSystem)
			return nullptr;

		Ref< const IrradianceGridResource > resource = instance->getObject< IrradianceGridResource >();
		if (!resource)
			return nullptr;

		Ref< IStream > stream = instance->readData(L"Data");
		if (!stream)
			return nullptr;

		return resource->createInstance(stream, m_renderSystem);
	}
	else
		return nullptr;
}

}
