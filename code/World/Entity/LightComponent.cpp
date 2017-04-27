/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "World/Entity.h"
#include "World/Entity/LightComponent.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.LightComponent", LightComponent, IEntityComponent)

LightComponent::LightComponent(
	LightType lightType,
	const Vector4& sunColor,
	const Vector4& baseColor,
	const Vector4& shadowColor,
	const resource::Proxy< render::ITexture >& probeDiffuseTexture,
	const resource::Proxy< render::ITexture >& probeSpecularTexture,
	const resource::Proxy< render::ITexture >& cloudShadowTexture,
	bool castShadow,
	float range,
	float radius,
	float flickerAmount,
	float flickerFilter
)
:	m_owner(0)
,	m_lightType(lightType)
,	m_sunColor(sunColor)
,	m_baseColor(baseColor)
,	m_shadowColor(shadowColor)
,	m_probeDiffuseTexture(probeDiffuseTexture)
,	m_probeSpecularTexture(probeSpecularTexture)
,	m_cloudShadowTexture(cloudShadowTexture)
,	m_castShadow(castShadow)
,	m_range(range)
,	m_radius(radius)
,	m_flickerAmount(flickerAmount)
,	m_flickerFilter(flickerFilter)
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
