/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Shape/Editor/Spline/SplineComponent.h"

#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Model/Model.h"
#include "Model/Operations/CalculateNormals.h"
#include "Model/Operations/CalculateTangents.h"
#include "Model/Operations/MergeModel.h"
#include "Physics/Body.h"
#include "Physics/Mesh.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/PhysicsManager.h"
#include "Physics/StaticBodyDesc.h"
#include "Render/Buffer.h"
#include "Render/Context/RenderContext.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/VertexElement.h"
#include "Resource/IResourceManager.h"
#include "Shape/Editor/Spline/ControlPointComponent.h"
#include "Shape/Editor/Spline/ControlPointComponentData.h"
#include "Shape/Editor/Spline/SplineComponentData.h"
#include "Shape/Editor/Spline/SplineLayerComponent.h"
#include "World/Entity.h"
#include "World/Entity/GroupComponent.h"
#include "World/IWorldRenderPass.h"
#include "World/World.h"
#include "World/WorldBuildContext.h"

#include <limits>

namespace traktor::shape
{
namespace
{

#pragma pack(1)

struct Vertex
{
	float position[3];
	float normal[4];
	float tangent[4];
	float binormal[4];
	float texCoord[2];
};

#pragma pack()

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SplineComponent", SplineComponent, world::IEntityComponent)

SplineComponent::SplineComponent(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	physics::PhysicsManager* physicsManager,
	const resource::Proxy< render::Shader >& defaultShader,
	const SplineComponentData* data)
	: m_resourceManager(resourceManager)
	, m_renderSystem(renderSystem)
	, m_physicsManager(physicsManager)
	, m_defaultShader(defaultShader)
	, m_data(data)
	, m_dirty(true)
{
}

SplineComponent::~SplineComponent()
{
	// Need to call destroy here since editor doesn't always call destroy on components
	// but instead rely on reference counting to do the cleanup.
	destroy();
}

void SplineComponent::destroy()
{
	if (m_updateJob)
	{
		m_updateJob->wait();
		m_updateJob = nullptr;
	}

	safeDestroy(m_rtwInstance);
	safeDestroy(m_indexBuffer);
	safeDestroy(m_vertexBuffer);
}

void SplineComponent::setOwner(world::Entity* owner)
{
	m_owner = owner;
}

void SplineComponent::setWorld(world::World* world)
{
	if (world != m_world)
	{
		if (m_updateJob)
		{
			m_updateJob->wait();
			m_updateJob = nullptr;
		}

		safeDestroy(m_rtwInstance);
		m_world = world;
		m_dirty = true;
	}
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
	if (m_updateJob != nullptr)
	{
		// Waiting for update of spline, check if job is done.
		if (m_updateJob->wait(0))
		{
			m_updateJob = nullptr;

			if (m_updateJobModel)
			{
				// Create the collision body.
				{
					safeDestroy(m_body);

					AlignedVector< Vector4 > positions;
					positions.reserve(m_updateJobModel->getVertices().size());
					for (const auto& vertex : m_updateJobModel->getVertices())
					{
						const Vector4 position = m_updateJobModel->getPosition(vertex.getPosition());
						positions.push_back(position.xyz1());
					}

					AlignedVector< physics::Mesh::Triangle > triangles;
					triangles.reserve(m_updateJobModel->getPolygons().size());
					for (const auto& polygon : m_updateJobModel->getPolygons())
					{
						T_FATAL_ASSERT(polygon.getVertexCount() == 3);
						triangles.push_back(
							{ {
								  polygon.getVertex(0),
								  polygon.getVertex(1),
								  polygon.getVertex(2),
							  },
								0 });
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
						T_FILE_LINE_W);
					if (m_body)
						m_body->setEnable(true);
				}

				// Create runtime render mesh from model.
				{
					m_batches.resize(0);

					const uint32_t nvertices = m_updateJobModel->getVertexCount();
					const uint32_t nindices = m_updateJobModel->getPolygonCount() * 3;

					if (nvertices > 0 && nindices > 0)
					{
						if (m_vertexBuffer == nullptr || m_vertexBuffer->getBufferSize() < nvertices * sizeof(Vertex))
						{
							safeDestroy(m_vertexBuffer);

							AlignedVector< render::VertexElement > vertexElements;
							vertexElements.push_back(render::VertexElement(render::DataUsage::Position, render::DtFloat3, offsetof(Vertex, position)));
							vertexElements.push_back(render::VertexElement(render::DataUsage::Normal, render::DtFloat4, offsetof(Vertex, normal)));
							vertexElements.push_back(render::VertexElement(render::DataUsage::Tangent, render::DtFloat4, offsetof(Vertex, tangent)));
							vertexElements.push_back(render::VertexElement(render::DataUsage::Binormal, render::DtFloat4, offsetof(Vertex, binormal)));
							vertexElements.push_back(render::VertexElement(render::DataUsage::Custom, render::DtFloat2, offsetof(Vertex, texCoord)));
							m_vertexLayout = m_renderSystem->createVertexLayout(vertexElements);

							m_vertexBuffer = m_renderSystem->createBuffer(
								render::BuVertex,
								(nvertices + 4 * 128) * sizeof(Vertex),
								false);
						}

						Vertex* vertex = (Vertex*)m_vertexBuffer->lock();
						for (const auto& v : m_updateJobModel->getVertices())
						{
							const Vector4 p = m_updateJobModel->getPosition(v.getPosition());
							const Vector4 n = (v.getNormal() != model::c_InvalidIndex) ? m_updateJobModel->getNormal(v.getNormal()).xyz0() : Vector4::zero();
							const Vector4 t = (v.getTangent() != model::c_InvalidIndex) ? m_updateJobModel->getNormal(v.getTangent()).xyz0() : Vector4::zero();
							const Vector4 b = (v.getBinormal() != model::c_InvalidIndex) ? m_updateJobModel->getNormal(v.getBinormal()).xyz0() : Vector4::zero();
							const Vector2 uv = (v.getTexCoord(0) != model::c_InvalidIndex) ? m_updateJobModel->getTexCoord(v.getTexCoord(0)) : Vector2::zero();

							p.storeUnaligned(vertex->position);
							n.storeUnaligned(vertex->normal);
							t.storeUnaligned(vertex->tangent);
							b.storeUnaligned(vertex->binormal);

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
						for (uint32_t i = 0; i < m_updateJobModel->getMaterialCount(); ++i)
						{
							uint32_t count = 0;
							for (const auto& p : m_updateJobModel->getPolygons())
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

							const model::Material material = m_updateJobModel->getMaterial(i);
							const uint32_t materialHash = DeepHash(&material).get();
							const Guid materialId = Guid(L"{8BB018D2-7AAC-4F9D-A5A4-DE396604862C}").permutation(materialHash);

							if (!m_resourceManager->bind(resource::Id< render::Shader >(materialId), batch.shader))
								batch.shader = m_defaultShader;

							batch.primitives = render::Primitives::setIndexed(
								render::PrimitiveType::Triangles,
								offset,
								count);

							offset += count * 3;
						}
						m_indexBuffer->unlock();

						world::RTWorldComponent* rtw = m_world ? m_world->getComponent< world::RTWorldComponent >() : nullptr;
						if (rtw != nullptr && nindices > 0)
						{
							safeDestroy(m_rtwInstance);

							Ref< render::Buffer > vertexAttributes = m_renderSystem->createBuffer(render::BuStructured, m_updateJobModel->getPolygonCount() * 3 * sizeof(world::HWRT_Material), false);
							world::HWRT_Material* vptr = (world::HWRT_Material*)vertexAttributes->lock();

							for (uint32_t i = 0; i < m_updateJobModel->getMaterialCount(); ++i)
							{
								for (const auto& polygon : m_updateJobModel->getPolygonsByMaterial(i))
								{
									const auto& material = m_updateJobModel->getMaterial(polygon.getMaterial());

									Vector4 albedo = material.getColor();
									for (const auto& vertex : polygon.getVertices())
									{
										const uint32_t colorId = m_updateJobModel->getVertex(vertex).getColor();
										if (colorId != model::c_InvalidIndex)
										{
											albedo = m_updateJobModel->getColor(colorId);
											break;
										}
									}

									for (uint32_t j = 0; j < 3; ++j)
									{
										const auto& vertex = m_updateJobModel->getVertex(polygon.getVertex(j));

										m_updateJobModel->getNormal(vertex.getNormal()).storeUnaligned3(vptr->normal);

										if (vertex.getColor() != model::c_InvalidIndex)
											m_updateJobModel->getColor(vertex.getColor()).storeUnaligned3(vptr->albedo);
										else
											albedo.storeUnaligned3(vptr->albedo);

										vptr->emissive = material.getEmissive();

										vptr->texCoord[0] = vptr->texCoord[1] = 0.0f;
										vptr->albedoMap = -1;

										++vptr;
									}
								}
							}

							vertexAttributes->unlock();

							AlignedVector< render::Primitives > primitives;
							primitives.push_back(render::Primitives::setIndexed(
								render::PrimitiveType::Triangles,
								0,
								nindices / 3));

							Ref< render::IAccelerationStructure > blas = m_renderSystem->createAccelerationStructure(m_vertexBuffer, m_vertexLayout, m_indexBuffer, render::IndexType::UInt32, primitives);
							if (blas != nullptr)
								m_rtwInstance = rtw->createInstance(blas, vertexAttributes);
						}
					}
					else
					{
						safeDestroy(m_vertexBuffer);
						safeDestroy(m_indexBuffer);
						safeDestroy(m_rtwInstance);
					}
				}
			}
			else
			{
				safeDestroy(m_vertexBuffer);
				safeDestroy(m_indexBuffer);
				safeDestroy(m_rtwInstance);
			}
		}
	}
	else
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
		if (!controlPointsDirty)
			return;

		m_path = TransformPath();
		for (uint32_t i = 0; i < controlPoints.size(); ++i)
		{
			const Transform T = controlPoints[i]->getTransform();
			TransformPath::Key k;
			k.T = (float)i / (controlPoints.size() - 1);
			k.position = T.translation();
			k.orientation = T.rotation().toEulerAngles();
			k.values[0] = controlPoints[i]->getData()->getScale();
			k.values[1] = controlPoints[i]->getData()->getAutomaticOrientationWeight();
			m_path.insert(k);
		}

		m_updateJobModel = nullptr;
		m_updateJob = JobManager::getInstance().add([controlPoints, this]() {
			for (auto component : m_owner->getComponents())
			{
				if (const auto layer = dynamic_type_cast< const SplineLayerComponent* >(component))
				{
					Ref< model::Model > layerModel = layer->createModel(m_path, m_data->isClosed(), true);
					if (!layerModel)
						continue;

					if (m_updateJobModel)
						m_updateJobModel->apply(model::MergeModel(*layerModel, Transform::identity(), 0.01f));
					else
						m_updateJobModel = layerModel;
				}
			}
			if (m_updateJobModel)
			{
				m_updateJobModel->apply(model::CalculateNormals(false));
				m_updateJobModel->apply(model::CalculateTangents(false));
			}
		});

		m_dirty = false;
	}
}

void SplineComponent::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass)
{
	if (!m_indexBuffer || !m_vertexBuffer)
		return;

	auto renderContext = context.getRenderContext();
	for (const auto& batch : m_batches)
	{
		auto sp = worldRenderPass.getProgram(batch.shader);
		if (!sp)
			continue;

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
			renderBlock->programParams);

		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(
			sp.priority,
			renderBlock);
	}
}

}
