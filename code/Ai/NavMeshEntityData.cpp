/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ai/NavMesh.h"
#include "Ai/NavMeshEntityData.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace ai
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ai.NavMeshEntityData", 0, NavMeshEntityData, world::EntityData)

void NavMeshEntityData::serialize(ISerializer& s)
{
	world::EntityData::serialize(s);
	s >> resource::Member< NavMesh >(L"navMesh", m_navMesh);
}

	}
}
