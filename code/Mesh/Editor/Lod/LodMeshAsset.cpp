/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
