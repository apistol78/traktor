#if !defined(WINCE)
#	include <ctime>
#else
#	include <time_ce.h>
#endif
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
	float flickerFilter
)
:	m_owner(0)
,	m_lightType(lightType)
,	m_color(color)
,	m_castShadow(castShadow)
,	m_range(range)
,	m_radius(radius)
,	m_flickerAmount(flickerAmount)
,	m_flickerFilter(flickerFilter)
,	m_flickerValue(0.0f)
,	m_flickerCoeff(0.0f)
#if !defined(WINCE)
,	m_random(uint32_t(clock()))
#else
,	m_random(uint32_t(clock_ce()))
#endif
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
	m_flickerValue = m_random.nextFloat() * (1.0f - m_flickerFilter) + m_flickerValue * m_flickerFilter;
	m_flickerCoeff = 1.0f - m_flickerValue * m_flickerAmount;
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
