#include "Animation/PathEntity/PathEntityData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberEnum.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.PathEntityData", 0, PathEntityData, world::EntityData)

PathEntityData::PathEntityData()
:	m_timeMode(PathEntity::TmManual)
{
}

Ref< PathEntity > PathEntityData::createEntity(const world::IEntityBuilder* builder) const
{
	Ref< world::Entity > entity = builder->create(m_entityData);
	return new PathEntity(
		getTransform(),
		m_path,
		m_timeMode,
		entity
	);
}

void PathEntityData::serialize(ISerializer& s)
{
	const MemberEnum< PathEntity::TimeMode >::Key c_TimeMode_Keys[] =
	{
		{ L"TmManual", PathEntity::TmManual },
		{ L"TmOnce", PathEntity::TmOnce },
		{ L"TmLoop", PathEntity::TmLoop },
		{ L"TmPingPong", PathEntity::TmPingPong },
		{ 0 }
	};

	world::EntityData::serialize(s);

	s >> MemberComposite< TransformPath >(L"path", m_path);
	s >> MemberEnum< PathEntity::TimeMode >(L"timeMode", m_timeMode, c_TimeMode_Keys);
	s >> MemberRef< world::EntityData >(L"entityData", m_entityData);
}

	}
}
