#include "Core/Math/Random.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Heightfield/Heightfield.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Terrain/RubbleEntityData.h"
#include "Terrain/Terrain.h"
#include "World/IEntityBuilder.h"
#include "World/Entity/GroupEntity.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.RubbleEntityData", 0, RubbleEntityData, world::EntityData)

RubbleEntityData::RubbleEntityData()
:	m_count(0)
{
}

Ref< world::Entity > RubbleEntityData::createEntity(resource::IResourceManager* resourceManager, const world::IEntityBuilder* builder) const
{
	resource::Proxy< Terrain > terrain;

	if (!resourceManager->bind(m_terrain, terrain))
		return 0;

	const resource::Proxy< hf::Heightfield >& heightfield = terrain->getHeightfield();
	T_ASSERT (heightfield);

	const Vector4& worldExtent = heightfield->getWorldExtent();
	Random random;

	Ref< world::GroupEntity > groupEntity = new world::GroupEntity(getTransform());
	for (uint32_t i = 0; i < m_count; ++i)
	{
		float x = (random.nextFloat() - 0.5f) * worldExtent.x();
		float z = (random.nextFloat() - 0.5f) * worldExtent.z();

		float y = heightfield->getWorldHeight(x, z);

		Ref< world::Entity > entity = builder->create(m_entityData);
		if (!entity)
			return 0;

		entity->setTransform(Transform(
			Vector4(x, y, z, 1.0f)
		));

		groupEntity->addEntity(entity);
	}

	return groupEntity;
}

void RubbleEntityData::serialize(ISerializer& s)
{
	world::EntityData::serialize(s);

	s >> resource::Member< Terrain >(L"terrain", m_terrain);
	s >> MemberRef< world::EntityData >(L"entityData", m_entityData);
	s >> Member< uint32_t >(L"count", m_count);
}

	}
}
