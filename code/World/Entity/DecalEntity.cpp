#include "World/Entity/DecalEntity.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.DecalEntity", DecalEntity, Entity)

DecalEntity::DecalEntity(
	const Transform& transform,
	float size,
	float thickness,
	float alpha,
	float cullDistance,
	const resource::Proxy< render::Shader >& shader
)
:	m_transform(transform)
,	m_size(size)
,	m_thickness(thickness)
,	m_alpha(alpha)
,	m_cullDistance(cullDistance)
,	m_age(0.0f)
,	m_shader(shader)
{
}

void DecalEntity::setTransform(const Transform& transform)
{
	m_transform = transform;
}

bool DecalEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Aabb3 DecalEntity::getBoundingBox() const
{
	return Aabb3(
		Vector4(-m_size, -m_thickness, -m_size, 1.0f),
		Vector4(m_size, m_thickness, m_size, 1.0f)
	);
}

void DecalEntity::update(const UpdateParams& update)
{
	m_age += update.deltaTime;
}

	}
}
