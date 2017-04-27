/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/Indoor/IndoorMesh.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Context/RenderContext.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.IndoorMesh", IndoorMesh, IMesh)

IndoorMesh::IndoorMesh()
{
}

const Aabb3& IndoorMesh::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

bool IndoorMesh::supportTechnique(render::handle_t technique) const
{
	return true;
}

void IndoorMesh::render(
	render::RenderContext* renderContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
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
	std::set< int > activeSectors;

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
			std::set< int > visibleSectors;
			for (std::set< int >::iterator i = activeSectors.begin(); i != activeSectors.end(); ++i)
				findVisibleSectors(
					frustum,
					worldRenderView.getView(),
					*i,
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

	const std::vector< render::Mesh::Part >& meshParts = m_mesh->getParts();

	for (std::set< int >::iterator i = activeSectors.begin(); i != activeSectors.end(); ++i)
	{
		Sector& sector = m_sectors[*i];
		
		SmallMap< render::handle_t, std::vector< Part > >::const_iterator it = sector.parts.find(worldRenderPass.getTechnique());
		if (it == sector.parts.end())
			continue;

		for (std::vector< Part >::const_iterator j = it->second.begin(); j != it->second.end(); ++j)
		{
			m_shader->setTechnique(j->shaderTechnique);

			worldRenderPass.setShaderCombination(
				m_shader,
				worldTransform,
				getBoundingBox()
			);

			render::IProgram* program = m_shader->getCurrentProgram();
			if (!program)
				continue;

			render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >("IndoorMesh");

			renderBlock->distance = distance;
			renderBlock->program = program;
			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->indexBuffer = m_mesh->getIndexBuffer();
			renderBlock->vertexBuffer = m_mesh->getVertexBuffer();
			renderBlock->primitives = meshParts[j->meshPart].primitives;

			renderBlock->programParams->beginParameters(renderContext);
			worldRenderPass.setProgramParameters(
				renderBlock->programParams,
				worldTransform,
				worldTransform,
				getBoundingBox()
			);
			if (parameterCallback)
				parameterCallback->setParameters(renderBlock->programParams);
			renderBlock->programParams->endParameters(renderContext);

			renderContext->draw(
				m_shader->getCurrentPriority(),
				renderBlock
			);
		}
	}
}

void IndoorMesh::findVisibleSectors(
	const AlignedVector< Plane >& frustum,
	const Matrix44& view,
	int currentSector,
	std::set< int >& outVisibleSectors
)
{
	for (AlignedVector< Portal >::const_iterator i = m_portals.begin(); i != m_portals.end(); ++i)
	{
		if (i->sectorA == currentSector || i->sectorB == currentSector)
		{
			int nextSector = (i->sectorA == currentSector) ? i->sectorB : i->sectorA;
			if (outVisibleSectors.find(nextSector) != outVisibleSectors.end())
				continue;

			Winding3 clipped;
			for (Winding3::points_t::const_iterator j = i->winding.getPoints().begin(); j != i->winding.getPoints().end(); ++j)
				clipped.push(
					view * *j
				);

			Plane clippedPlane;
			if (clipped.getPlane(clippedPlane) && clippedPlane.normal().z() >= 0.0f)
				clipped.flip();

			for (AlignedVector< Plane >::const_iterator j = frustum.begin(); j != frustum.end(); ++j)
			{
				Winding3 front, back;
				clipped.split(*j, front, back);
				clipped = front;

				if (clipped.size() < 3)
					break;
			}

			if (clipped.size() >= 3)
			{
				outVisibleSectors.insert(nextSector);

				AlignedVector< Plane > nextFrustum(clipped.size());
				for (size_t j = 0, k = clipped.size() - 1; j < clipped.size(); k = j++)
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
}
