/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/BillboardComponentRenderer.h"

#include "Core/Misc/ObjectStore.h"
#include "Render/Buffer.h"
#include "Render/Context/RenderContext.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/VertexElement.h"
#include "World/Entity/BillboardComponent.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

#include <cmath>

namespace traktor::world
{
namespace
{

#pragma pack(1)
struct BillboardVertex
{
	float position[3];
	float texCoord[2];
};
#pragma pack()

/*! Number of quads which fit in a frame's vertex buffer. */
const uint32_t c_maxBillboards = 4096;

/*! Ratio between a quad's side and the sphere it sweep as it turn. */
const float c_boundingRadiusFactor = 0.70710678f * 2.0f;

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.BillboardComponentRenderer", 0, BillboardComponentRenderer, IEntityRenderer)

BillboardComponentRenderer::BillboardComponentRenderer(render::IRenderSystem* renderSystem)
{
	initialize(renderSystem);
}

bool BillboardComponentRenderer::initialize(const ObjectStore& objectStore)
{
	return initialize(objectStore.get< render::IRenderSystem >());
}

const TypeInfoSet BillboardComponentRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< BillboardComponent >();
}

void BillboardComponentRenderer::setup(
	const WorldSetupContext& context,
	const WorldRenderView& worldRenderView,
	const AlignedVector< Object* >& renderables)
{
	m_batches.resize(0);
	m_quadCount = 0;

	if (renderables.empty() || m_vertexBuffers[0] == nullptr)
		return;

	// Cycle buffers so the quads of a frame still in flight are left alone.
	m_cycle = (m_cycle + 1) % sizeof_array(m_vertexBuffers);

	BillboardVertex* vertex = static_cast< BillboardVertex* >(m_vertexBuffers[m_cycle]->lock());
	if (!vertex)
		return;

	const Vector4& eyePosition = worldRenderView.getEyePosition();
	const Matrix44& view = worldRenderView.getView();
	const Frustum& cullFrustum = worldRenderView.getCullFrustum();

	for (Object* renderable : renderables)
	{
		if (m_quadCount >= c_maxBillboards)
			break;

		const BillboardComponent* billboardComponent = static_cast< const BillboardComponent* >(renderable);

		const render::Shader* shader = billboardComponent->getShader();
		if (shader == nullptr)
			continue;

		const Transform& transform = billboardComponent->getTransform();
		const Vector4 center = transform * billboardComponent->getOffset().xyz1();

		// Below the start distance the entity is expected to be drawn in full, beyond the
		// cull distance not at all.
		const Vector4 toEye = (eyePosition - center).xyz0();
		const Scalar distance = toEye.length();
		if (distance < Scalar(billboardComponent->getStartDistance()) || distance > Scalar(billboardComponent->getCullDistance()))
			continue;

		const Scalar radius(billboardComponent->getSize() * 0.5f * c_boundingRadiusFactor);
		if (cullFrustum.inside(view * center.xyz1(), radius) == Frustum::Result::Outside)
			continue;

		// Orient the quad along the atlas view rendered closest to how it is being looked at.
		const BillboardComponent::View& v = billboardComponent->findView(transform.inverse() * (-toEye));

		const Scalar half(billboardComponent->getSize() * 0.5f);
		const Vector4 right = (transform * v.right) * half;
		const Vector4 up = (transform * v.up) * half;

		const Vector4 corners[] = {
			center - right + up,
			center + right + up,
			center + right - up,
			center - right - up
		};
		const float texCoords[][2] = {
			{ v.texCoord.x(), v.texCoord.y() },
			{ v.texCoord.z(), v.texCoord.y() },
			{ v.texCoord.z(), v.texCoord.w() },
			{ v.texCoord.x(), v.texCoord.w() }
		};

		for (int32_t i = 0; i < 4; ++i)
		{
			vertex->position[0] = corners[i].x();
			vertex->position[1] = corners[i].y();
			vertex->position[2] = corners[i].z();
			vertex->texCoord[0] = texCoords[i][0];
			vertex->texCoord[1] = texCoords[i][1];
			vertex++;
		}

		// Extend the open batch when it draw with the same shader, which is the common
		// case of many instances of a single kind of entity.
		if (m_batches.empty() || m_batches.back().shader != shader)
			m_batches.push_back({ shader, m_quadCount * 6, 0 });

		m_batches.back().count += 2;
		m_quadCount++;
	}

	m_vertexBuffers[m_cycle]->unlock();
}

void BillboardComponentRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	const AlignedVector< Object* >& renderables)
{
	if (m_batches.empty())
		return;

	render::RenderContext* renderContext = context.getRenderContext();
	T_ASSERT(renderContext);

	for (const auto& batch : m_batches)
	{
		const auto sp = worldRenderPass.getProgram(batch.shader);
		if (!sp)
			continue;

		render::IndexedRenderBlock* renderBlock = renderContext->allocNamed< render::IndexedRenderBlock >(L"Billboard");

		renderBlock->distance = 0.0f;
		renderBlock->program = sp.program;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->indexBuffer = m_indexBuffer->getBufferView();
		renderBlock->indexType = render::IndexType::UInt16;
		renderBlock->vertexBuffer = m_vertexBuffers[m_cycle]->getBufferView();
		renderBlock->vertexLayout = m_vertexLayout;
		renderBlock->primitive = render::PrimitiveType::Triangles;
		renderBlock->offset = batch.offset;
		renderBlock->count = batch.count;

		renderBlock->programParams->beginParameters(renderContext);

		// Quads are already in world space, so there is no per object transform to speak of.
		worldRenderPass.setProgramParameters(
			renderBlock->programParams,
			Transform::identity(),
			Transform::identity());

		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(sp.priority, renderBlock);
	}
}

bool BillboardComponentRenderer::initialize(render::IRenderSystem* renderSystem)
{
	if (!renderSystem)
		return false;

	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DataUsage::Position, render::DtFloat3, offsetof(BillboardVertex, position), 0));
	vertexElements.push_back(render::VertexElement(render::DataUsage::Custom, render::DtFloat2, offsetof(BillboardVertex, texCoord), 0));
	T_ASSERT_M(render::getVertexSize(vertexElements) == sizeof(BillboardVertex), L"Incorrect size of vertex");
	m_vertexLayout = renderSystem->createVertexLayout(vertexElements);

	for (uint32_t i = 0; i < sizeof_array(m_vertexBuffers); ++i)
	{
		m_vertexBuffers[i] = renderSystem->createBuffer(render::BuVertex, c_maxBillboards * 4 * sizeof(BillboardVertex), true, T_FILE_LINE_W);
		if (!m_vertexBuffers[i])
			return false;
	}

	m_indexBuffer = renderSystem->createBuffer(render::BuIndex, c_maxBillboards * 6 * sizeof(uint16_t), false, T_FILE_LINE_W);
	if (!m_indexBuffer)
		return false;

	uint16_t* index = static_cast< uint16_t* >(m_indexBuffer->lock());
	if (!index)
		return false;

	for (uint32_t i = 0; i < c_maxBillboards; ++i)
	{
		const uint16_t base = (uint16_t)(i * 4);
		*index++ = base + 0;
		*index++ = base + 1;
		*index++ = base + 2;
		*index++ = base + 0;
		*index++ = base + 2;
		*index++ = base + 3;
	}

	m_indexBuffer->unlock();
	return true;
}

}
