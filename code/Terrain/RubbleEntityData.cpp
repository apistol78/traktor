#include "Core/Math/Const.h"
#include "Core/Math/Random.h"
#include "Core/Serialization/AttributeRange.h"
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

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.RubbleEntityData", 1, RubbleEntityData, world::EntityData)

RubbleEntityData::RubbleEntityData()
:	m_count(0)
,	m_seed(5489UL)
,	m_alignToNormal(0.0f)
,	m_randomHeadAngle(true)
,	m_groundOffset(0.0f)
,	m_spreadDistance(0.0f)
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
	Random random(m_seed);

	std::vector< Vector2 > positions;
	positions.reserve(m_count);

	Ref< world::GroupEntity > groupEntity = new world::GroupEntity(getTransform());
	for (uint32_t i = 0; i < m_count; ++i)
	{
		float x = (random.nextFloat() - 0.5f) * worldExtent.x();
		float z = (random.nextFloat() - 0.5f) * worldExtent.z();

		if (m_spreadDistance > FUZZY_EPSILON)
		{
			bool accept = true;
			for (std::vector< Vector2 >::const_iterator j = positions.begin(); j != positions.end(); ++j)
			{
				float d = (*j - Vector2(x, z)).length2();
				if (d < m_spreadDistance * m_spreadDistance)
				{
					accept = false;
					break;
				}
			}
			if (!accept)
				continue;

			positions.push_back(Vector2(x, z));
		}

		float y = heightfield->getWorldHeight(x, z);

		const float c_distance = 0.5f;
		float yn[] =
		{
			heightfield->getWorldHeight(x + c_distance, z),
			heightfield->getWorldHeight(x - c_distance, z),
			heightfield->getWorldHeight(x, z + c_distance),
			heightfield->getWorldHeight(x, z - c_distance)
		};

		Vector4 center(x, y, z, 1.0f);
		Vector4 axisX = (Vector4(x + c_distance, yn[0], z) - Vector4(x - c_distance, yn[1], z)).normalized();
		Vector4 axisZ = (Vector4(x, yn[2], z + c_distance) - Vector4(x, yn[3], z - c_distance)).normalized();
		Vector4 normal = cross(axisX, axisZ).normalized();

		Ref< world::Entity > entity = builder->create(m_entityData);
		if (!entity)
			return 0;

		Transform T = Transform(center + Vector4(0.0f, m_groundOffset, 0.0f));

		if (m_alignToNormal > FUZZY_EPSILON)
		{
			float ax = std::asin(axisX.y()) * m_alignToNormal;
			float az = std::asin(axisZ.y()) * m_alignToNormal;

			Transform Tx(Quaternion::fromAxisAngle(Vector4(-az, 0.0f, 0.0f)));
			Transform Tz(Quaternion::fromAxisAngle(Vector4(0.0f, 0.0f, ax)));

			T = T * Tx * Tz;
		}

		if (m_randomHeadAngle)
			T = T * Transform(Quaternion::fromAxisAngle(Vector4(0.0f, random.nextFloat() * TWO_PI, 0.0f)));

		entity->setTransform(T);

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

	if (s.getVersion() >= 1)
	{
		s >> Member< uint32_t >(L"seed", m_seed);
		s >> Member< float >(L"alignToNormal", m_alignToNormal, AttributeRange(0.0f, 1.0f));
		s >> Member< bool >(L"randomHeadAngle", m_randomHeadAngle);
		s >> Member< float >(L"groundOffset", m_groundOffset);
		s >> Member< float >(L"spreadDistance", m_spreadDistance);
	}
}

	}
}
