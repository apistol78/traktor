#include "Animation/PathEntity/PathComponentData.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberEnum.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.PathComponentData", 0, PathComponentData, world::IEntityComponentData)

Ref< PathComponent > PathComponentData::createComponent() const
{
	return new PathComponent(m_path, m_timeMode, m_timeOffset);
}

void PathComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void PathComponentData::serialize(ISerializer& s)
{
	const MemberEnum< PathComponent::TimeMode >::Key c_TimeMode_Keys[] =
	{
		{ L"TmManual", PathComponent::TmManual },
		{ L"TmOnce", PathComponent::TmOnce },
		{ L"TmLoop", PathComponent::TmLoop },
		{ L"TmPingPong", PathComponent::TmPingPong },
		{ 0 }
	};

	s >> MemberComposite< TransformPath >(L"path", m_path);
	s >> MemberEnum< PathComponent::TimeMode >(L"timeMode", m_timeMode, c_TimeMode_Keys);
	s >> Member< float >(L"timeOffset", m_timeOffset, AttributeUnit(UnitType::Seconds));
}

	}
}
