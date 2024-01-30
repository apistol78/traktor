/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/Entity/DecalComponent.h"
#include "World/Entity/DecalRenderer.h"

namespace traktor::world
{
	namespace
	{

#pragma pack(1)
struct Vertex
{
	float position[3];
};
#pragma pack()

const uint32_t c_maxRenderDecals = 32;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.DecalRenderer", DecalRenderer, IEntityRenderer)

DecalRenderer::DecalRenderer(render::IRenderSystem* renderSystem)
{
	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DataUsage::Position, render::DtFloat3, offsetof(Vertex, position), 0));
	T_ASSERT_M (render::getVertexSize(vertexElements) == sizeof(Vertex), L"Incorrect size of vertex");
	m_vertexLayout = renderSystem->createVertexLayout(vertexElements);

	m_vertexBuffer = renderSystem->createBuffer(render::BuVertex, 8 * sizeof(Vertex), false);
	T_ASSERT_M (m_vertexBuffer, L"Unable to create vertex buffer");

	Vector4 extents[8];
	Aabb3(Vector4(-1.0f, -1.0f, -1.0f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)).getExtents(extents);

	Vertex* vertex = static_cast< Vertex* >(m_vertexBuffer->lock());
	T_ASSERT(vertex);

	for (uint32_t i = 0; i < sizeof_array(extents); ++i)
	{
		vertex->position[0] = extents[i].x();
		vertex->position[1] = extents[i].y();
		vertex->position[2] = extents[i].z();
		vertex++;
	}

	m_vertexBuffer->unlock();

	m_indexBuffer = renderSystem->createBuffer(render::BuIndex, 6 * 2 * 3 * sizeof(uint16_t), false);
	T_ASSERT_M (m_indexBuffer, L"Unable to create index buffer");

	const int32_t* faces = Aabb3::getFaces();

	uint16_t* index = static_cast< uint16_t* >(m_indexBuffer->lock());
	for (uint32_t i = 0; i < 6; ++i)
	{
		*index++ = faces[i * 4 + 0];
		*index++ = faces[i * 4 + 1];
		*index++ = faces[i * 4 + 3];

		*index++ = faces[i * 4 + 1];
		*index++ = faces[i * 4 + 2];
		*index++ = faces[i * 4 + 3];
	}

	m_indexBuffer->unlock();
}

const TypeInfoSet DecalRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< DecalComponent >();
}

void DecalRenderer::setup(
	const WorldSetupContext& context
)
{
}

void DecalRenderer::setup(
	const WorldSetupContext& context,
	const WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void DecalRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	const DecalComponent* decalComponent = static_cast< const DecalComponent* >(renderable);
	const Transform& transform = decalComponent->getTransform();

	const float s = decalComponent->getSize();
	const float t = decalComponent->getThickness();
	const float d = decalComponent->getCullDistance();

	const Vector4 center = worldRenderView.getView() * transform.translation().xyz1();
	if (center.length2() > d * d)
		return;

	const Scalar radius = Scalar(std::sqrt(s * s + s * s + t * t));
	if (worldRenderView.getCullFrustum().inside(center, radius) == Frustum::Result::Outside)
		return;

	m_decalComponents.push_back(decalComponent);
}

void DecalRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass
)
{
	render::RenderContext* renderContext = context.getRenderContext();
	T_ASSERT(renderContext);

	const Matrix44& projection = worldRenderView.getProjection();
	const Matrix44& view = worldRenderView.getView();

	const Scalar p11 = projection.get(0, 0);
	const Scalar p22 = projection.get(1, 1);
	const Vector4 magicCoeffs(1.0f / p11, 1.0f / p22, 0.0f, 0.0f);

	// Render all decal boxes.
	const uint32_t decalsCount = std::min< uint32_t >(uint32_t(m_decalComponents.size()), c_maxRenderDecals);
	for (uint32_t i = 0; i < decalsCount; ++i)
	{
		const DecalComponent* decalComponent = m_decalComponents[i];
		T_ASSERT(decalComponent);

		const render::Shader* shader = decalComponent->getShader();
		T_ASSERT(shader);

		const auto sp = worldRenderPass.getProgram(shader);
		if (!sp)
			continue;

		const Transform& transform = decalComponent->getTransform();

		const Matrix44 worldView = view * transform.toMatrix44();
		const Matrix44 worldViewInv = worldView.inverse();

		render::IndexedRenderBlock* renderBlock = renderContext->allocNamed< render::IndexedRenderBlock >(L"Decal");

		renderBlock->distance = 0.0f;
		renderBlock->program = sp.program;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->indexBuffer = m_indexBuffer->getBufferView();
		renderBlock->indexType = render::IndexType::UInt16;
		renderBlock->vertexLayout = m_vertexLayout;
		renderBlock->vertexBuffer = m_vertexBuffer->getBufferView();
		renderBlock->primitive = render::PrimitiveType::Triangles;
		renderBlock->offset = 0;
		renderBlock->count = 12;
		renderBlock->minIndex = 0;
		renderBlock->maxIndex = 7;

		renderBlock->programParams->beginParameters(renderContext);

		worldRenderPass.setProgramParameters(
			renderBlock->programParams,
			transform,
			transform
		);

		renderBlock->programParams->setVectorParameter(s_handleDecalParams, Vector4(
			decalComponent->getSize(),
			decalComponent->getThickness(),
			decalComponent->getAlpha(),
			decalComponent->getAge()
		));
		renderBlock->programParams->setVectorParameter(s_handleMagicCoeffs, magicCoeffs);
		renderBlock->programParams->setMatrixParameter(s_handleWorldViewInv, worldViewInv);
		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(render::RenderPriority::PostOpaque, renderBlock);
	}

	// Flush all queued decals.
	m_decalComponents.resize(0);
}

}
