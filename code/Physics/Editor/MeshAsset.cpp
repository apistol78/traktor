/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Physics/Editor/MeshAsset.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.MeshAsset", 2, MeshAsset, editor::Asset)

MeshAsset::MeshAsset()
:	m_calculateConvexHull(true)
,	m_margin(0.04f)
{
}

void MeshAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	if (s.getVersion() >= 1)
		s >> Member< bool >(L"calculateConvexHull", m_calculateConvexHull);

	if (s.getVersion() >= 2)
		s >> Member< float >(L"margin", m_margin);
}

	}
}
