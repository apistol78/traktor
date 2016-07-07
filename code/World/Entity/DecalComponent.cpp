#include "World/Entity/DecalComponent.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.DecalComponent", DecalComponent, IEntityComponent)

DecalComponent::DecalComponent(
	float size,
	float thickness,
	float alpha,
	float cullDistance,
	const resource::Proxy< render::Shader >& shader
)
:	m_size(size)
,	m_thickness(thickness)
,	m_alpha(alpha)
,	m_cullDistance(cullDistance)
,	m_age(0.0f)
,	m_shader(shader)
{
}

void DecalComponent::destroy()
{
}

void DecalComponent::setOwner(Entity* owner)
{
}

void DecalComponent::setTransform(const Transform& transform)
{
	m_transform = transform;
}

Aabb3 DecalComponent::getBoundingBox() const
{
	return Aabb3(
		Vector4(-m_size, -m_thickness, -m_size, 1.0f),
		Vector4(m_size, m_thickness, m_size, 1.0f)
	);
}

void DecalComponent::update(const UpdateParams& update)
{
	m_age += update.deltaTime;
}

	}
}
