#include <cstring>
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

		Ref< render::StructBuffer > buffer = m_renderSystem->createStructBuffer(
			layout,
			render::getStructSize(layout) * 64 * 16 * 64
		);
		if (!buffer)
			return nullptr;

		IrradianceGridData* grid = (IrradianceGridData*)buffer->lock();
		T_FATAL_ASSERT(grid);

		for (int32_t iy = 0; iy < 16; ++iy)
		{
			for (int32_t ix = 0; ix < 64; ++ix)
			{
				for (int32_t iz = 0; iz < 64; ++iz)
				{
					int32_t cell = (iy * 64 * 64) + (iz * 64) + ix;
					
					auto& g = grid[cell];
					std::memset(&g, 0, sizeof(IrradianceGridData));

					if ((ix + iy + iz) & 1)
						g.shR0_3[0] = 1.0f;
					else
						g.shG0_3[0] = 1.0f;
				}
			}
		}

		buffer->unlock();

		return new IrradianceGrid(
			Vector4(64, 16, 64, 0),
			buffer
		);
	}
	else
		return nullptr;
}

	}
}
