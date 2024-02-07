/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Misc/SafeDestroy.h"
#include "Model/Model.h"
#include "Model/Operations/MergeModel.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Shape/Editor/Spline/ControlPointComponent.h"
#include "Shape/Editor/Spline/SplineComponent.h"
#include "Shape/Editor/Spline/SplineComponentData.h"
#include "Shape/Editor/Spline/SplineLayerComponent.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/Entity.h"
#include "World/Entity/GroupComponent.h"

#include "Physics/Body.h"
#include "Physics/Mesh.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/PhysicsManager.h"
#include "Physics/StaticBodyDesc.h"

namespace traktor::shape
{
	namespace
	{

#pragma pack(1)
struct Vertex
{
	float position[3];
	float normal[3];
	float texCoord[2];
};
#pragma pack()

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SplineComponent", SplineComponent, world::IEntityComponent)

SplineComponent::SplineComponent(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	physics::PhysicsManager* physicsManager,
	const resource::Proxy< render::Shader >& shader,
	const SplineComponentData* data
)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
,	m_physicsManager(physicsManager)
,	m_shader(shader)
,	m_data(data)
,	m_dirty(true)
{
}

void SplineComponent::destroy()
{
}

void SplineComponent::setOwner(world::Entity* owner)
{
	m_owner = owner;
}

void SplineComponent::setTransform(const Transform& transform)
{
}

Aabb3 SplineComponent::getBoundingBox() const
{
	return Aabb3();
}

void SplineComponent::update(const world::UpdateParams& update)
{
	// Fetch group component; contain all control point entities.
	auto group = m_owner->getComponent< world::GroupComponent >();
	if (!group)
		return;

	// Get control points.
	RefArray< ControlPointComponent > controlPoints;
	for (auto entity : group->getEntities())
	{
		auto controlPoint = entity->getComponent< ControlPointComponent >();
		if (controlPoint)
			controlPoints.push_back(controlPoint);
	}

	// Check if any control point is dirty.
	bool controlPointsDirty = m_dirty;
	for (auto controlPoint : controlPoints)
	{
		if (controlPoint->checkDirty())
		{
			controlPointsDirty |= true;
			break;
		}
	}

	// Update transform path if any control point is dirty.
	if (controlPointsDirty)
	{
		m_path = TransformPath();
		for (uint32_t i = 0; i < controlPoints.size(); ++i)
		{
			const Transform T = controlPoints[i]->getTransform();
			TransformPath::Key k;
			k.T = (float)i / (controlPoints.size() - 1);
			k.position = T.translation();
			k.orientation = T.rotation().toEulerAngles();
			k.values[0] = controlPoints[i]->getScale();
			m_path.insert(k);
		}

		// Generate geometry from path.
		Ref< model::Model > outputModel;
		for (auto component : m_owner->getComponents())
		{
			if (const auto layer = dynamic_type_cast< const SplineLayerComponent* >(component))
			{
				Ref< model::Model > layerModel = layer->createModel(m_path, m_data->isClosed(), true);
				if (!layerModel)
					continue;

				if (outputModel)
				{
					model::MergeModel merge(*layerModel, Transform::identity(), 0.01f);
					merge.apply(*outputModel);
				}
				else
					outputModel = layerModel;
			}
		}

		// In case no layers has been added yet.
		if (!outputModel)
		{
			safeDestroy(m_vertexBuffer);
			safeDestroy(m_indexBuffer);	
			m_dirty = false;
			return;
		}

		// Create the collision body.
		{
			safeDestroy(m_body);

			AlignedVector< Vector4 > positions;
			for (const auto& vertex : outputModel->getVertices())
			{
				const Vector4 position = outputModel->getPosition(vertex.getPosition());
				positions.push_back(position.xyz1());
			}

			AlignedVector< physics::Mesh::Triangle > triangles;
			for (const auto& polygon : outputModel->getPolygons())
			{
				T_FATAL_ASSERT(polygon.getVertexCount() == 3);
				triangles.push_back(
					{
						{
							polygon.getVertex(0),
							polygon.getVertex(1),
							polygon.getVertex(2),
						},
						0
					}
				);
			}

			AlignedVector< physics::Mesh::Material > materials;
			materials.push_back({ 0.5f, 0.5f });

			Ref< physics::Mesh > mesh = new physics::Mesh();
			mesh->setVertices(positions);
			mesh->setShapeTriangles(triangles);
			mesh->setMaterials(materials);

			Ref< physics::ShapeDesc > shapeDesc = new physics::ShapeDesc();
			shapeDesc->setCollisionGroup(m_data->getCollisionGroup());
			shapeDesc->setCollisionMask(m_data->getCollisionMask());

			Ref< physics::StaticBodyDesc > bodyDesc = new physics::StaticBodyDesc(shapeDesc);

			m_body = m_physicsManager->createBody(
				m_resourceManager,
				bodyDesc,
				mesh,
				T_FILE_LINE_W
			);
			if (m_body)
				m_body->setEnable(true);
		}

		// Create runtime render mesh from model.
		{
			m_batches.resize(0);

			const uint32_t nvertices = outputModel->getVertexCount();
			const uint32_t nindices = outputModel->getPolygonCount() * 3;

			if (nvertices > 0 && nindices > 0)
			{
				if (m_vertexBuffer == nullptr || m_vertexBuffer->getBufferSize() < nvertices * sizeof(Vertex))
				{
					safeDestroy(m_vertexBuffer);

					AlignedVector< render::VertexElement > vertexElements;
					vertexElements.push_back(render::VertexElement(render::DataUsage::Position, render::DtFloat3, offsetof(Vertex, position)));
					vertexElements.push_back(render::VertexElement(render::DataUsage::Normal, render::DtFloat3, offsetof(Vertex, normal)));
					vertexElements.push_back(render::VertexElement(render::DataUsage::Custom, render::DtFloat2, offsetof(Vertex, texCoord)));
					m_vertexLayout = m_renderSystem->createVertexLayout(vertexElements);

					m_vertexBuffer = m_renderSystem->createBuffer(
						render::BuVertex,
						(nvertices + 4 * 128) * sizeof(Vertex),
						false
					);
				}

				Vertex* vertex = (Vertex*)m_vertexBuffer->lock();
				for (const auto& v : outputModel->getVertices())
				{
					const Vector4 p = outputModel->getPosition(v.getPosition());
					const Vector4 n = (v.getNormal() != model::c_InvalidIndex) ? outputModel->getNormal(v.getNormal()) : Vector4::zero();
					const Vector2 uv = (v.getTexCoord(0) != model::c_InvalidIndex) ? outputModel->getTexCoord(v.getTexCoord(0)) : Vector2::zero();

					p.storeUnaligned(vertex->position);
					n.storeUnaligned(vertex->normal);

					vertex->texCoord[0] = uv.x;
					vertex->texCoord[1] = uv.y;

					++vertex;
				}
				m_vertexBuffer->unlock();

				// Create indices and material batches.
				if (m_indexBuffer == nullptr || m_indexBuffer->getBufferSize() < nindices * sizeof(uint32_t))
				{
					safeDestroy(m_indexBuffer);
					m_indexBuffer = m_renderSystem->createBuffer(render::BuIndex, (nindices + 3 * 128) * sizeof(uint32_t), false);
				}

				uint32_t* index = (uint32_t*)m_indexBuffer->lock();
				uint32_t offset = 0;
				for (uint32_t i = 0; i < outputModel->getMaterialCount(); ++i)
				{
					uint32_t count = 0;
					for (const auto& p : outputModel->getPolygons())
					{
						if (p.getMaterial() == i)
						{
							*index++ = (uint32_t)p.getVertex(0);
							*index++ = (uint32_t)p.getVertex(1);
							*index++ = (uint32_t)p.getVertex(2);
							++count;
						}
					}

					if (!count)
						continue;

					auto& batch = m_batches.push_back();
					batch.primitives.setIndexed(
						render::PrimitiveType::Triangles,
						offset,
						count,
						0,
						nvertices - 1
					);

					offset += count * 3;
				}
				m_indexBuffer->unlock();
			}
			else
			{
				safeDestroy(m_vertexBuffer);
				safeDestroy(m_indexBuffer);
			}
		}

		m_dirty = false;
	}
}

void SplineComponent::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	if (!m_indexBuffer || !m_vertexBuffer)
		return;

	auto sp = worldRenderPass.getProgram(m_shader);
	if (!sp)
		return;

	auto renderContext = context.getRenderContext();
	for (const auto& batch : m_batches)
	{
		auto renderBlock = renderContext->allocNamed< render::SimpleRenderBlock >(L"Solid");
		renderBlock->distance = std::numeric_limits< float >::max();
		renderBlock->program = sp.program;
		renderBlock->indexBuffer = m_indexBuffer->getBufferView();
		renderBlock->indexType = render::IndexType::UInt32;
		renderBlock->vertexBuffer = m_vertexBuffer->getBufferView();
		renderBlock->vertexLayout = m_vertexLayout;
		renderBlock->primitives = batch.primitives;

		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->programParams->beginParameters(renderContext);

		worldRenderPass.setProgramParameters(
			renderBlock->programParams
		);

		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(
			sp.priority,
			renderBlock
		);
	}
}

}
