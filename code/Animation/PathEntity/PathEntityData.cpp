/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.PathEntityData", 1, PathEntityData, world::EntityData)

PathEntityData::PathEntityData()
:	m_timeMode(PathEntity::TmManual)
,	m_timeOffset(0.0f)
{
}

Ref< PathEntity > PathEntityData::createEntity(const world::IEntityBuilder* builder) const
{
	Ref< world::Entity > entity = builder->create(m_entityData);
	return new PathEntity(
		getTransform(),
		m_path,
		m_timeMode,
		m_timeOffset,
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

	if (s.getVersion< PathEntityData >() >= 1)
		s >> Member< float >(L"timeOffset", m_timeOffset);

	s >> MemberRef< world::EntityData >(L"entityData", m_entityData);
}

	}
}
