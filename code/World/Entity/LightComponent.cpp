#include "World/Entity.h"
#include "World/Entity/LightComponent.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.LightComponent", LightComponent, IEntityComponent)

LightComponent::LightComponent(
	LightType lightType,
	const Vector4& color,
	bool castShadow,
	float range,
	float radius,
	float flickerAmount,
	float flickerFilter,
	const render::SHCoeffs* shCoeffs
)
:	m_owner(0)
,	m_lightType(lightType)
,	m_color(color)
,	m_castShadow(castShadow)
,	m_range(range)
,	m_radius(radius)
,	m_flickerAmount(flickerAmount)
,	m_flickerFilter(flickerFilter)
,	m_shCoeffs(shCoeffs)
{
}

void LightComponent::destroy()
{
}

void LightComponent::setOwner(Entity* owner)
{
	m_owner = owner;
}

void LightComponent::update(const UpdateParams& update)
{
}

void LightComponent::setTransform(const Transform& transform)
{
}

Aabb3 LightComponent::getBoundingBox() const
{
	return Aabb3();
}

Transform LightComponent::getTransform() const
{
	Transform transform;
	m_owner->getTransform(transform);
	return transform;
}

	}
}
