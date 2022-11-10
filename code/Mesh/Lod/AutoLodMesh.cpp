/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/Lod/AutoLodMesh.h"
#include "Mesh/Static/StaticMesh.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.AutoLodMesh", AutoLodMesh, IMesh)

const Aabb3& AutoLodMesh::getBoundingBox(float lodDistance) const
{
	return m_boundingBox;
}

bool AutoLodMesh::supportTechnique(float lodDistance, render::handle_t technique) const
{
	//StaticMesh* staticMesh = getStaticMesh(lodDistance);
	//if (staticMesh)
	//	return staticMesh->supportTechnique(technique);
	//else
	//	return true;

	return false;
}

void AutoLodMesh::build(
	float lodDistance,
	render::RenderContext* renderContext,
	const world::IWorldRenderPass& worldRenderPass,
	const Transform& lastWorldTransform,
	const Transform& worldTransform,
	float distance,
	const IMeshParameterCallback* parameterCallback
)
{
	//StaticMesh* staticMesh = getStaticMesh(lodDistance);
	//if (staticMesh)
	//	staticMesh->build(
	//		renderContext,
	//		worldRenderPass,
	//		lastWorldTransform,
	//		worldTransform,
	//		distance,
	//		parameterCallback
	//	);
}

StaticMesh* AutoLodMesh::getStaticMesh(float lodDistance) const
{
	if (lodDistance >= m_cullDistance)
		return 0;

	int32_t lastLod = int32_t(m_lods.size()) - 1;
	int32_t index = clamp(int32_t(lodDistance * lastLod / m_maxDistance), 0, lastLod);

	return m_lods[index];
}

}
