/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ai/NavMeshEntity.h"

namespace traktor
{
	namespace ai
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ai.NavMeshEntity", NavMeshEntity, world::Entity)

NavMeshEntity::NavMeshEntity()
{
}

NavMeshEntity::NavMeshEntity(const resource::Proxy< NavMesh >& navMesh)
:	m_navMesh(navMesh)
{
}

Aabb3 NavMeshEntity::getBoundingBox() const
{
	return Aabb3();
}

void NavMeshEntity::update(const world::UpdateParams& update)
{
}

	}
}
