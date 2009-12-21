#include "Physics/ShapeDesc.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.ShapeDesc", ShapeDesc, ISerializable)

ShapeDesc::ShapeDesc()
:	m_localTransform(Transform::identity())
,	m_group(~0UL)
{
}

void ShapeDesc::setLocalTransform(const Transform& localTransform)
{
	m_localTransform = localTransform;
}

const Transform& ShapeDesc::getLocalTransform() const
{
	return m_localTransform;
}

void ShapeDesc::setGroup(uint32_t group)
{
	m_group = group;
}

uint32_t ShapeDesc::getGroup() const
{
	return m_group;
}

bool ShapeDesc::bind(resource::IResourceManager* resourceManager)
{
	return true;
}

bool ShapeDesc::serialize(ISerializer& s)
{
	s >> MemberComposite< Transform >(L"localTransform", m_localTransform);
	if (s.getVersion() >= 1)
		s >> Member< uint32_t >(L"group", m_group);
	return true;
}

	}
}
