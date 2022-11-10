/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Mesh/Partition/OctreeNodeData.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.OctreeNodeData", 0, OctreeNodeData, ISerializable)

void OctreeNodeData::setBoundingBox(const Aabb3& boundingBox)
{
	m_boundingBox = boundingBox;
}

void OctreeNodeData::addPartIndex(uint8_t worldTechniqueId, uint32_t partIndex)
{
	m_partIndices[worldTechniqueId].push_back(partIndex);
}

void OctreeNodeData::setChild(int32_t index, const OctreeNodeData* child)
{
	m_children[index] = child;
}

void OctreeNodeData::serialize(ISerializer& s)
{
	s >> MemberAabb3(L"boundingBox", m_boundingBox);
	s >> MemberSmallMap
		<
			uint8_t,
			AlignedVector< uint32_t >,
			Member< uint8_t >,
			MemberAlignedVector< uint32_t >
		>(L"partIndices", m_partIndices);
	s >> MemberStaticArray< Ref< const OctreeNodeData >, 8, MemberRef< const OctreeNodeData > >(L"children", m_children);
}

}
