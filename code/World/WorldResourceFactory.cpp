#include <cstring>
#include "Core/Io/Reader.h"
#include "Core/Log/Log.h"
#include "Database/Instance.h"
#include "Render/IRenderSystem.h"
#include "Render/StructBuffer.h"
#include "Render/StructElement.h"
#include "World/EntityData.h"
#include "World/EntityEventSet.h"
#include "World/EntityEventSetData.h"
#include "World/IEntityBuilder.h"
#include "World/IEntityEvent.h"
#include "World/IEntityEventData.h"
#include "World/IrradianceGrid.h"
#include "World/IrradianceGridResource.h"
#include "World/WorldResourceFactory.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldResourceFactory", WorldResourceFactory, resource::IResourceFactory)

WorldResourceFactory::WorldResourceFactory(render::IRenderSystem* renderSystem, const IEntityBuilder* entityBuilder)
:	m_renderSystem(renderSystem)
,	m_entityBuilder(entityBuilder)
{
}

const TypeInfoSet WorldResourceFactory::getResourceTypes() const
{
	TypeInfoSet typeInfoSet;
	typeInfoSet.insert< EntityData >();
	typeInfoSet.insert< EntityEventSetData >();
	typeInfoSet.insert< IEntityEventData >();
	typeInfoSet.insert< IrradianceGridResource >();
	return typeInfoSet;
}

const TypeInfoSet WorldResourceFactory::getProductTypes(const TypeInfo& resourceType) const
{
	if (is_type_a< EntityData >(resourceType))
		return makeTypeInfoSet< EntityData >();
	else if (is_type_a< EntityEventSetData >(resourceType))
		return makeTypeInfoSet< EntityEventSet >();
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
	else if (is_type_a< EntityEventSet >(productType))
	{
		if (!m_entityBuilder)
			return nullptr;

		Ref< const EntityEventSetData > eventSetData = instance->getObject< EntityEventSetData >();
		if (!eventSetData)
			return nullptr;

		Ref< EntityEventSet > eventSet = eventSetData->create(m_entityBuilder);
		if (!eventSet)
			return nullptr;

		return eventSet;
	}
	else if (is_type_a< IEntityEvent >(productType))
	{
		if (!m_entityBuilder)
			return nullptr;

		Ref< const IEntityEventData > eventData = instance->getObject< IEntityEventData >();
		if (!eventData)
			return nullptr;

		Ref< IEntityEvent > event = m_entityBuilder->create< IEntityEvent >(eventData);
		if (!event)
			return nullptr;

		return event;
	}
	else if (is_type_a< IrradianceGrid >(productType))
	{
		if (!m_renderSystem)
			return nullptr;

		AlignedVector< render::StructElement > layout;
		layout.push_back(render::StructElement(render::DtFloat4, offsetof(IrradianceGridData, shR0_3)));
		layout.push_back(render::StructElement(render::DtFloat4, offsetof(IrradianceGridData, shR4_7)));
		layout.push_back(render::StructElement(render::DtFloat4, offsetof(IrradianceGridData, shG0_3)));
		layout.push_back(render::StructElement(render::DtFloat4, offsetof(IrradianceGridData, shG4_7)));
		layout.push_back(render::StructElement(render::DtFloat4, offsetof(IrradianceGridData, shB0_3)));
		layout.push_back(render::StructElement(render::DtFloat4, offsetof(IrradianceGridData, shB4_7)));
		layout.push_back(render::StructElement(render::DtFloat4, offsetof(IrradianceGridData, shRGB_8)));
		T_FATAL_ASSERT(sizeof(IrradianceGridData) == render::getStructSize(layout));

		Ref< IrradianceGridResource > resource = instance->getObject< IrradianceGridResource >();
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

		Ref< render::StructBuffer > buffer = m_renderSystem->createStructBuffer(
			layout,
			render::getStructSize(layout) * size[0] * size[1] * size[2]
		);
		if (!buffer)
			return nullptr;

		IrradianceGridData* grid = (IrradianceGridData*)buffer->lock();
		T_FATAL_ASSERT(grid);

		for (int32_t x = 0; x < size[0]; ++x)
		{
			for (int32_t y = 0; y < size[1]; ++y)
			{
				for (int32_t z = 0; z < size[2]; ++z)
				{
					auto& g = *grid++;
					for (int32_t i = 0; i < 4; ++i)
					{
						reader >> g.shR0_3[i];
						reader >> g.shG0_3[i];
						reader >> g.shB0_3[i];
					}
					for (int32_t i = 0; i < 4; ++i)
					{
						reader >> g.shR4_7[i];
						reader >> g.shG4_7[i];
						reader >> g.shB4_7[i];
					}
					reader >> g.shRGB_8[0];
					reader >> g.shRGB_8[1];
					reader >> g.shRGB_8[2];
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
}
