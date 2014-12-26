#include "World/Entity/DirectionalLightEntity.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.DirectionalLightEntity", DirectionalLightEntity, Entity)

DirectionalLightEntity::DirectionalLightEntity(
	const Transform& transform,
	const Vector4& sunColor,
	const Vector4& baseColor,
	const Vector4& shadowColor,
	const resource::Proxy< render::ITexture >& cloudShadowTexture,
	bool castShadow
)
:	m_transform(transform)
,	m_sunColor(sunColor)
,	m_baseColor(baseColor)
,	m_shadowColor(shadowColor)
,	m_cloudShadowTexture(cloudShadowTexture)
,	m_castShadow(castShadow)
{
}

void DirectionalLightEntity::update(const UpdateParams& update)
{
}

void DirectionalLightEntity::setTransform(const Transform& transform)
{
	m_transform = transform;
}

bool DirectionalLightEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Aabb3 DirectionalLightEntity::getBoundingBox() const
{
	return Aabb3(Vector4(-1.0f, -1.0f, -1.0f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f));
}

	}
}
