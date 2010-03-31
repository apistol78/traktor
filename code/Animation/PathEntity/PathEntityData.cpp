#include "Animation/PathEntity/PathEntityData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberEnum.h"
#include "World/Entity/IEntityBuilder.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.PathEntityData", 0, PathEntityData, world::SpatialEntityData)

PathEntityData::PathEntityData()
:	m_timeMode(PathEntity::TmManual)
{
}

Ref< PathEntity > PathEntityData::createEntity(world::IEntityBuilder* builder) const
{
	Ref< world::SpatialEntity > entity = dynamic_type_cast< world::SpatialEntity* >(builder->create(m_entityData));
	return new PathEntity(
		getTransform(),
		m_path,
		m_timeMode,
		entity
	);
}

bool PathEntityData::serialize(ISerializer& s)
{
	const MemberEnum< PathEntity::TimeMode >::Key c_TimeMode_Keys[] =
	{
		{ L"TmManual", PathEntity::TmManual },
		{ L"TmOnce", PathEntity::TmOnce },
		{ L"TmLoop", PathEntity::TmLoop },
		{ L"TmPingPong", PathEntity::TmPingPong },
		{ 0 }
	};

	if (!world::SpatialEntityData::serialize(s))
		return false;

	s >> MemberComposite< TransformPath >(L"path", m_path);
	s >> MemberEnum< PathEntity::TimeMode >(L"timeMode", m_timeMode, c_TimeMode_Keys);
	s >> MemberRef< world::SpatialEntityData >(L"entityData", m_entityData);

	return true;
}

	}
}
