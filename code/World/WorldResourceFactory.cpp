/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Io/Reader.h"
#include "Core/Log/Log.h"
#include "Database/Instance.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
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

		Reader reader(stream);

		uint32_t version;
		reader >> version;
		if (version != 2)
		{
			log::error << L"Unable to read irradiance grid, unknown version " << version << Endl;
			return nullptr;
		}

		IrradianceGrid::gridSize_t size;
		reader >> size[0];
		reader >> size[1];
		reader >> size[2];

		float mn[3], mx[3];
		reader >> mn[0];
		reader >> mn[1];
		reader >> mn[2];
		reader >> mx[0];
		reader >> mx[1];
		reader >> mx[2];

		Ref< render::Buffer > buffer = m_renderSystem->createBuffer(
			render::BuStructured,
			size[0] * size[1] * size[2] * sizeof(IrradianceGridData),
			false
		);
		if (!buffer)
			return nullptr;

		IrradianceGridData* grid = (IrradianceGridData*)buffer->lock();
		T_FATAL_ASSERT(grid);

		float tmp[3];
		for (uint32_t x = 0; x < size[0]; ++x)
		{
			for (uint32_t y = 0; y < size[1]; ++y)
			{
				for (uint32_t z = 0; z < size[2]; ++z)
				{
					auto& g = *grid++;
					for (int32_t i = 0; i < 4; ++i)
					{
						reader >> tmp[0];
						reader >> tmp[1];
						reader >> tmp[2];
						g.shR0_3[i] = tmp[0];
						g.shG0_3[i] = tmp[1];
						g.shB0_3[i] = tmp[2];
					}
					for (int32_t i = 0; i < 4; ++i)
					{
						reader >> tmp[0];
						reader >> tmp[1];
						reader >> tmp[2];
						g.shR4_7[i] = tmp[0];
						g.shG4_7[i] = tmp[1];
						g.shB4_7[i] = tmp[2];
					}
					reader >> tmp[0];
					reader >> tmp[1];
					reader >> tmp[2];
					g.shRGB_8[0] = tmp[0];
					g.shRGB_8[1] = tmp[1];
					g.shRGB_8[2] = tmp[2];
				}
			}
		}

		buffer->unlock();

		return new IrradianceGrid(
			size,
			Aabb3(Vector4(mn[0], mn[1], mn[2]), Vector4(mx[0], mx[1], mx[2])),
			buffer
		);
	}
	else
		return nullptr;
}

}
