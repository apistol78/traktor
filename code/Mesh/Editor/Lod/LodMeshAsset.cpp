/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/Lod/LodMeshAsset.h"

namespace traktor
{
	namespace mesh
	{
	
T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.mesh.LodMeshAsset", 0, LodMeshAsset, ISerializable)

LodMeshAsset::LodMeshAsset()
:	m_maxDistance(10.0f)
,	m_cullDistance(20.0f)
{
}

void LodMeshAsset::serialize(ISerializer& s)
{
	s >> Member< float >(L"maxDistance", m_maxDistance);
	s >> Member< float >(L"cullDistance", m_cullDistance);
	s >> MemberStlList< Guid >(L"lods", m_lods, AttributeType(type_of< MeshAsset >()));
}

	}
}
