/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Mesh/Partition/OctreeNode.h"
#include "Mesh/Partition/OctreePartition.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.OctreePartition", OctreePartition, IPartition)

OctreePartition::OctreePartition(OctreeNode* node, const AlignedVector< render::handle_t >& worldTechniques)
:	m_node(node)
,	m_worldTechniques(worldTechniques)
{
}

void OctreePartition::traverse(
	const Frustum& frustum,
	const Matrix44& worldView,
	render::handle_t worldTechnique,
	AlignedVector< uint32_t >& outPartIndices
) const
{
	auto it = std::find(m_worldTechniques.begin(), m_worldTechniques.end(), worldTechnique);
	if (it == m_worldTechniques.end())
		return;

	uint8_t worldTechniqueId = (uint8_t)std::distance(m_worldTechniques.begin(), it);

	Matrix44 worldViewInv = worldView.inverse();
	Frustum frustumObject;

	for (int32_t i = 0; i < 6; ++i)
		frustumObject.planes[i] = worldViewInv * frustum.planes[i];

	m_node->traverse(frustumObject, worldTechniqueId, outPartIndices);
}

}
