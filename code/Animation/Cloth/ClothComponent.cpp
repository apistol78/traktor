/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/SkeletonComponent.h"
#include "Animation/Cloth/Cloth.h"
#include "Animation/Cloth/ClothComponent.h"
#include "Core/Math/Const.h"
#include "Core/Math/Plane.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/JobManager.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "World/Entity.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"

namespace traktor::animation
{
	namespace
	{

struct ClothVertex
{
	float position[4];
	float normal[4];
	float texCoord[2];
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.ClothComponent", ClothComponent, world::IEntityComponent)

bool ClothComponent::create(
	render::IRenderSystem* renderSystem,
	const resource::Proxy< render::Shader >& shader,
	const resource::Proxy< Cloth >& cloth,
	float jointRadius,
	float damping,
	uint32_t solverIterations
)
{
	m_cloth = cloth;

	m_nodes.reserve(cloth->m_nodes.size());
	for (const auto& node : cloth->m_nodes)
	{
		auto& n = m_nodes.push_back();
		n.jointName = 0;
		n.position[0] =
		n.position[1] = node.position;
		n.texCoord = node.texCoord;
		n.invMass = Scalar(node.invMass);
	}

	m_solverIterations = solverIterations;

	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DataUsage::Position, render::DtFloat4, offsetof(ClothVertex, position)));
	vertexElements.push_back(render::VertexElement(render::DataUsage::Normal, render::DtFloat4, offsetof(ClothVertex, normal)));
	vertexElements.push_back(render::VertexElement(render::DataUsage::Custom, render::DtFloat2, offsetof(ClothVertex, texCoord)));
	m_vertexLayout = renderSystem->createVertexLayout(vertexElements);

	m_vertexBuffer = renderSystem->createBuffer(render::BuVertex, m_nodes.size() * sizeof(ClothVertex), true);
	if (!m_vertexBuffer)
		return false;

	m_triangleCount = cloth->m_triangles.size() / 3;

	m_indexBuffer = renderSystem->createBuffer(render::BuIndex, cloth->m_triangles.size() * sizeof(uint16_t), false);
	if (!m_indexBuffer)
		return false;

	uint16_t* index = static_cast< uint16_t* >(m_indexBuffer->lock());
	T_ASSERT(index);

	for (auto t : cloth->m_triangles)
		*index++ = (uint16_t)t;

	m_indexBuffer->unlock();

	m_damping = Scalar(1.0f - damping);
	m_jointRadius = Scalar(jointRadius);
	m_shader = shader;
	return true;
}

void ClothComponent::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	if (!m_shader->hasTechnique(worldRenderPass.getTechnique()))
		return;

	synchronize();

	if (
		m_updateRequired &&
		(worldRenderPass.getPassFlags() & world::IWorldRenderPass::First) != 0
	)
	{
		ClothVertex* vertexFront = static_cast< ClothVertex* >(m_vertexBuffer->lock());
		T_ASSERT(vertexFront);

		for (uint32_t i = 0; i < (uint32_t)m_nodes.size(); ++i)
		{
			const Cloth::Node& cn = m_cloth->m_nodes[i];
			const Node& node = m_nodes[i];

			const Vector4& p = node.position[0];

			Vector4 nf = Vector4::zero();
			if (cn.east != -1 && cn.north != -1)
			{
				const Vector4& nx = m_nodes[cn.east].position[0];
				const Vector4& ny = m_nodes[cn.north].position[0];
				nf = cross(ny - p, nx - p).normalized();
			}

			p.storeUnaligned(vertexFront->position);
			nf.storeUnaligned(vertexFront->normal);
			vertexFront->texCoord[0] = node.texCoord.x;
			vertexFront->texCoord[1] = node.texCoord.y;
			vertexFront++;
		}

		m_vertexBuffer->unlock();
		m_updateRequired = false;
	}

	auto sp = worldRenderPass.getProgram(m_shader);
	if (!sp)
		return;

	render::RenderContext* renderContext = context.getRenderContext();
	T_ASSERT(renderContext);

	auto renderBlock = renderContext->allocNamed< render::IndexedRenderBlock >(L"Cloth");
	renderBlock->distance = 0.0f;
	renderBlock->program = sp.program;
	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->indexBuffer = m_indexBuffer->getBufferView();
	renderBlock->indexType = render::IndexType::UInt16;
	renderBlock->vertexBuffer = m_vertexBuffer->getBufferView();
	renderBlock->vertexLayout = m_vertexLayout;
	renderBlock->primitive = render::PrimitiveType::Triangles;
	renderBlock->offset = 0;
	renderBlock->count = m_triangleCount;

	renderBlock->programParams->beginParameters(renderContext);
	worldRenderPass.setProgramParameters(
		renderBlock->programParams,
		m_transform,
		m_transform
	);
	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(
		sp.priority,
		renderBlock
	);
}

void ClothComponent::destroy()
{
	safeDestroy(m_vertexBuffer);
	safeDestroy(m_indexBuffer);
}

void ClothComponent::setOwner(world::Entity* owner)
{
	if (owner != m_owner)
	{
		m_owner = owner;
		
		// Reset time and update.
		m_time = 4.0f;
		m_updateTime = 0.0f;
		m_updateRequired = true;

		// Reset node positions.
		if (m_cloth != nullptr)
		{
			for (size_t i = 0; i < m_nodes.size(); ++i)
			{
				auto& n = m_nodes[i];
				n.position[0] =
				n.position[1] = m_cloth->m_nodes[i].position;
			}
		}
	}
}

void ClothComponent::setTransform(const Transform& transform)
{
	m_transform = transform;
}

Aabb3 ClothComponent::getBoundingBox() const
{
	return m_aabb;
}

void ClothComponent::update(const world::UpdateParams& update)
{
	const Transform transformInv = m_transform.inverse();
	const Vector4 gravity = transformInv * Vector4(0.0f, -1.0f, 0.0f, 0.0f);
	const Vector4 movement = m_lastPosition.w() > 0.0f ? transformInv * (m_transform.translation() - m_lastPosition).xyz0() : Vector4::zero();
	m_lastPosition = m_transform.translation().xyz1();

	m_updateClothJob = JobManager::getInstance().add([=, this](){
#if !defined(__IOS__)
		const float c_updateDeltaTime = 1.0f / 30.0f;
#else
		const float c_updateDeltaTime = 1.0f / 10.0f;
#endif
		const float c_timeScale = 4.0f;
		const Scalar c_stiffness = 0.001_simd;

		auto skeletonComponent = m_owner->getComponent< SkeletonComponent >();

		for (m_time += update.deltaTime * c_timeScale; m_updateTime < m_time; m_updateTime += c_updateDeltaTime)
		{
			m_aabb = Aabb3();
			for (auto& node : m_nodes)
			{
				if (node.jointName != 0 || node.invMass < Scalar(FUZZY_EPSILON))
					continue;

				const Vector4 force = gravity;
				const Vector4 current = node.position[0];
				const Vector4 velocity = current - node.position[1];

				node.position[0] += -movement * Scalar(c_updateDeltaTime) + velocity * m_damping + force * node.invMass * Scalar(c_updateDeltaTime * c_updateDeltaTime);
				node.position[1] = current;

				m_aabb.contain(node.position[0]);
			}

			for (uint32_t i = 0; i < m_solverIterations; ++i)
			{
				// Satisfy edge lengths.
				for (const auto& edge : m_cloth->m_edges)
				{
					const Vector4 delta = m_nodes[edge.indices[1]].position[0] - m_nodes[edge.indices[0]].position[0];
					const Scalar deltaLength = delta.length();
					if (deltaLength > FUZZY_EPSILON)
					{
						const Scalar diff = (deltaLength - Scalar(edge.length)) / deltaLength;
						m_nodes[edge.indices[0]].position[0] += delta * diff * m_nodes[edge.indices[0]].invMass * 0.5_simd;
						m_nodes[edge.indices[1]].position[0] -= delta * diff * m_nodes[edge.indices[1]].invMass * 0.5_simd;
					}
				}

				if (skeletonComponent)
				{
					// Ensure nodes are not inside joint spheres.
					const auto& poseTransforms = skeletonComponent->getPoseTransforms();
					for (auto& node : m_nodes)
					{
						for (const auto& poseTransform : poseTransforms)
						{
							const Vector4 sphereCenter = poseTransform.translation();
							const Scalar sphereRadius = m_jointRadius;
							const Vector4 d = (node.position[0] - sphereCenter).xyz0();
							if (dot3(d, d) <= sphereRadius * sphereRadius)
							{
								const Scalar depth = sphereRadius / d.length() - 1.0_simd;
								node.position[0] += d * depth;
							}
						}
					}

					// Ensure nodes are anchored.
					for (auto& node : m_nodes)
					{
						if (node.jointName == 0)
							continue;

						Transform poseTransform;
						if (skeletonComponent->getPoseTransform(node.jointName, poseTransform))
						{
							node.position[0] = (poseTransform.translation() + node.jointOffset).xyz1();
						}
					}
				}
			}

			m_updateRequired = true;
		}
	});
}

void ClothComponent::reset()
{
}

void ClothComponent::setNodeAnchor(render::handle_t jointName, const Vector4& jointOffset, uint32_t x, uint32_t y)
{
	//const uint32_t index = x + y * m_resolutionX;
	//if (index < m_resolutionX * m_resolutionY)
	//{
	//	m_nodes[index].jointName = jointName;
	//	m_nodes[index].jointOffset = jointOffset;
	//}
}

void ClothComponent::synchronize() const
{
	if (m_updateClothJob)
	{
		m_updateClothJob->wait();
		m_updateClothJob = nullptr;
	}
}

}
