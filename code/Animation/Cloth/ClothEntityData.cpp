#include "Animation/Cloth/ClothEntity.h"
#include "Animation/Cloth/ClothEntityData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.ClothEntityData", 0, ClothEntityData, world::EntityData)

ClothEntityData::ClothEntityData()
:	m_resolutionX(10)
,	m_resolutionY(10)
,	m_scale(1.0f)
,	m_solverIterations(4)
,	m_damping(0.01f)
{
}

Ref< ClothEntity > ClothEntityData::createEntity(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem
) const
{
	resource::Proxy< render::Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return 0;

	Ref< ClothEntity > clothEntity = new ClothEntity();
	if (!clothEntity->create(
		renderSystem,
		shader,
		m_resolutionX,
		m_resolutionY,
		m_scale,
		m_damping,
		m_solverIterations
	))
		return 0;

	// Fixate nodes by setting infinite mass.
	for (std::vector< Anchor >::const_iterator i = m_anchors.begin(); i != m_anchors.end(); ++i)
		clothEntity->setNodeInvMass(i->x, i->y, 0.0f);

	clothEntity->setTransform(getTransform());

	return clothEntity;
}

void ClothEntityData::serialize(ISerializer& s)
{
	world::EntityData::serialize(s);

	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> Member< uint32_t >(L"resolutionX", m_resolutionX);
	s >> Member< uint32_t >(L"resolutionY", m_resolutionY);
	s >> Member< float >(L"scale", m_scale);
	s >> MemberStlVector< Anchor, MemberComposite< Anchor > >(L"anchors", m_anchors);
	s >> Member< uint32_t >(L"solverIterations", m_solverIterations);
	s >> Member< float >(L"damping", m_damping);
}

void ClothEntityData::Anchor::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"x", x);
	s >> Member< uint32_t >(L"y", y);
}

	}
}
