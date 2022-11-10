/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/Indoor/IndoorMesh.h"
#include "Render/Buffer.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Context/RenderContext.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldRenderView.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.IndoorMesh", IndoorMesh, IMesh)

const Aabb3& IndoorMesh::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

bool IndoorMesh::supportTechnique(render::handle_t technique) const
{
	return true;
}

void IndoorMesh::build(
	render::RenderContext* renderContext,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	const Transform& worldTransform,
	float distance,
	const IMeshParameterCallback* parameterCallback
)
{
	Vector4 cameraPosition = worldRenderView.getView().inverse().translation();

	// Create initial clipper frustum, clipper frustums can have more than
	// 6 planes as it's later reconstructed from clipped portals.
	AlignedVector< Plane > frustum(6);
	for (int i = 0; i < 6; ++i)
		frustum[i] = worldRenderView.getCullFrustum().planes[i];

	// Find initially active sectors which are the sectors that the camera is within,
	// as sector bounding boxes are lousily calculated more than one sector
	// can become initially active.
	SmallSet< int > activeSectors;

	static bool searchActiveSectors = true;
	if (searchActiveSectors)
	{
		for (int i = 0; i < int(m_sectors.size()); ++i)
		{
			const Sector& sector = m_sectors[i];
			if (sector.boundingBox.inside(cameraPosition))
				activeSectors.insert(i);
		}
		if (!activeSectors.empty())
		{
			SmallSet< int > visibleSectors;
			for (const auto activeSector : activeSectors)
				findVisibleSectors(
					frustum,
					worldRenderView.getView(),
					activeSector,
					visibleSectors
				);

			activeSectors.insert(visibleSectors.begin(), visibleSectors.end());
		}
	}
	if (activeSectors.empty())
	{
		for (int i = 0; i < int(m_sectors.size()); ++i)
			activeSectors.insert(i);
	}

	// Render sectors, should probably sort all visible parts by their shader as
	// it will otherwise be a lot of state changes.

	const AlignedVector< render::Mesh::Part >& meshParts = m_mesh->getParts();

	for (const auto activeSector : activeSectors)
	{
		Sector& sector = m_sectors[activeSector];

		SmallMap< render::handle_t, std::vector< Part > >::const_iterator it = sector.parts.find(worldRenderPass.getTechnique());
		if (it == sector.parts.end())
			continue;

		for (const auto& part : it->second)
		{
			auto permutation = worldRenderPass.getPermutation(m_shader);
			permutation.technique = part.shaderTechnique;
			auto sp = m_shader->getProgram(permutation);
			if (!sp)
				continue;

			render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >(L"IndoorMesh");
			renderBlock->distance = distance;
			renderBlock->program = sp.program;
			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->indexBuffer = m_mesh->getIndexBuffer()->getBufferView();
			renderBlock->indexType = m_mesh->getIndexType();
			renderBlock->vertexBuffer = m_mesh->getVertexBuffer()->getBufferView();
			renderBlock->vertexLayout = m_mesh->getVertexLayout();
			renderBlock->primitives = meshParts[part.meshPart].primitives;

			renderBlock->programParams->beginParameters(renderContext);
			worldRenderPass.setProgramParameters(
				renderBlock->programParams,
				worldTransform,
				worldTransform
			);
			if (parameterCallback)
				parameterCallback->setParameters(renderBlock->programParams);
			renderBlock->programParams->endParameters(renderContext);

			renderContext->draw(
				sp.priority,
				renderBlock
			);
		}
	}
}

void IndoorMesh::findVisibleSectors(
	const AlignedVector< Plane >& frustum,
	const Matrix44& view,
	int currentSector,
	SmallSet< int >& outVisibleSectors
)
{
	for (const auto& portal : m_portals)
	{
		if (portal.sectorA == currentSector || portal.sectorB == currentSector)
		{
			int nextSector = (portal.sectorA == currentSector) ? portal.sectorB : portal.sectorA;
			if (outVisibleSectors.find(nextSector) != outVisibleSectors.end())
				continue;

			Winding3 clipped;
			for (const auto& point : portal.winding.get())
				clipped.push(
					view * point
				);

			Plane clippedPlane;
			if (clipped.getPlane(clippedPlane) && clippedPlane.normal().z() >= 0.0f)
				clipped.flip();

			for (const auto& plane : frustum)
			{
				Winding3 front, back;
				clipped.split(plane, front, back);
				clipped = front;

				if (clipped.size() < 3)
					break;
			}

			if (clipped.size() >= 3)
			{
				outVisibleSectors.insert(nextSector);

				AlignedVector< Plane > nextFrustum(clipped.size());
				for (uint32_t j = 0, k = clipped.size() - 1; j < clipped.size(); k = j++)
				{
					nextFrustum[j] = Plane(
						Vector4(0.0f, 0.0f, 0.0f, 1.0f),
						clipped[j],
						clipped[k]
					);
				}

				findVisibleSectors(nextFrustum, view, nextSector, outVisibleSectors);
			}
		}
	}
}

}
