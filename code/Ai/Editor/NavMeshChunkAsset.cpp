/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ai/Editor/NavMeshChunkAsset.h"

#include "Ai/Editor/NavMeshAsset.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor::ai
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ai.NavMeshChunkAsset", 0, NavMeshChunkAsset, ISerializable)

void NavMeshChunkAsset::Terrain::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"terrain", terrain);
	s >> MemberComposite< Transform >(L"transform", transform);
}

void NavMeshChunkAsset::Modifier::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"id", id);
	s >> MemberAabb3(L"bounds", bounds);
	s >> Member< int32_t >(L"area", area);
}

void NavMeshChunkAsset::serialize(ISerializer& s)
{
	s >> MemberRef< NavMeshAsset >(L"navMesh", m_navMesh);
	s >> Member< int32_t >(L"chunkX", m_chunkX);
	s >> Member< int32_t >(L"chunkZ", m_chunkZ);
	s >> Member< int32_t >(L"chunkTiles", m_chunkTiles);
	s >> Member< Vector4 >(L"origin", m_origin);
	s >> Member< float >(L"oceanHeight", m_oceanHeight);
	s >> Member< bool >(L"oceanClip", m_oceanClip);
	s >> MemberAlignedVector< Guid >(L"entities", m_entities);
	s >> MemberAlignedVector< Terrain, MemberComposite< Terrain > >(L"terrains", m_terrains);
	s >> MemberAlignedVector< Modifier, MemberComposite< Modifier > >(L"modifiers", m_modifiers);
	s >> Member< uint32_t >(L"stamp", m_stamp);
}

}
