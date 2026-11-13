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
#include "Render/ITexture.h"
#include "Render/Shader.h"
#include "Render/VertexElement.h"
#include "World/Entity/BillboardComponent.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor::world
{
namespace
{

#pragma pack(1)
struct BillboardVertex
{
	float position[3];
	float normal[3];
	float tangent[3];
	float binormal[3];
	float texCoord[2];
};
#pragma pack()

const uint32_t c_maxBillboards = 4096;

const render::Handle s_handleBillboardAlbedo(L"Billboard_Albedo");
const render::Handle s_handleBillboardNormals(L"Billboard_Normals");

void writeVector(float* out, const Vector4& v)
{
	out[0] = v.x();
	out[1] = v.y();
	out[2] = v.z();
}

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
	ViewState& viewState = m_viewStates[worldRenderView.getIndex()];
	viewState.vertexBufferView = nullptr;
	viewState.batches.resize(0);

	if (renderables.empty() || m_vertexBuffer == nullptr)
		return;

	BillboardVertex* vertex = static_cast< BillboardVertex* >(m_vertexBuffer->lock());
	if (!vertex)
		return;

	const Vector4& eyePosition = worldRenderView.getEyePosition();
	const Matrix44& view = worldRenderView.getView();
	const Frustum& cullFrustum = worldRenderView.getCullFrustum();

	uint32_t quadCount = 0;
	for (Object* renderable : renderables)
	{
		if (quadCount >= c_maxBillboards)
			break;

		const BillboardComponent* billboardComponent = static_cast< const BillboardComponent* >(renderable);

		const render::Shader* shader = billboardComponent->getShader();
		render::ITexture* albedoTexture = billboardComponent->getAlbedoTexture();
		render::ITexture* normalTexture = billboardComponent->getNormalTexture();
		if (shader == nullptr || albedoTexture == nullptr || normalTexture == nullptr)
			continue;

		const Transform& transform = billboardComponent->getTransform();
		const Vector4 center = transform * billboardComponent->getOffset().xyz1();

		const Scalar half(billboardComponent->getSize() * 0.5f);
		if (cullFrustum.inside(view * center, half * Scalar(1.4142f)) == Frustum::Result::Outside)
			continue;

		// Orient quad along the atlas view rendered closest to how it is being looked at.
		const BillboardComponent::View& v = billboardComponent->findView(transform.inverse() * (center - eyePosition).xyz0());

		const Vector4 right = transform * v.right;
		const Vector4 up = transform * v.up;
		const Vector4 normal = -(transform * v.forward);

		const Vector4 corners[] = {
			center - right * half + up * half,
			center + right * half + up * half,
			center + right * half - up * half,
			center - right * half - up * half
		};
		const float texCoords[][2] = {
			{ v.texCoord.x(), v.texCoord.y() },
			{ v.texCoord.z(), v.texCoord.y() },
			{ v.texCoord.z(), v.texCoord.w() },
			{ v.texCoord.x(), v.texCoord.w() }
		};

		for (int32_t i = 0; i < 4; ++i)
		{
			writeVector(vertex->position, corners[i]);
			writeVector(vertex->normal, normal);
			writeVector(vertex->tangent, right);
			writeVector(vertex->binormal, up);
			vertex->texCoord[0] = texCoords[i][0];
			vertex->texCoord[1] = texCoords[i][1];
			vertex++;
		}

		// Extend open batch when drawn with same shader and atlas.
		if (
			viewState.batches.empty() ||
			viewState.batches.back().shader != shader ||
			viewState.batches.back().albedoTexture != albedoTexture ||
			viewState.batches.back().normalTexture != normalTexture)
			viewState.batches.push_back({ shader, albedoTexture, normalTexture, quadCount * 6, 0 });

		viewState.batches.back().count += 2;
		quadCount++;
	}

	m_vertexBuffer->unlock();
	viewState.vertexBufferView = m_vertexBuffer->getBufferView();
}

void BillboardComponentRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	const AlignedVector< Object* >& renderables)
{
	const auto it = m_viewStates.find(worldRenderView.getIndex());
	if (it == m_viewStates.end() || it->second.vertexBufferView == nullptr)
		return;

	const ViewState& viewState = it->second;
	render::RenderContext* renderContext = context.getRenderContext();

	for (const auto& batch : viewState.batches)
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
		renderBlock->vertexBuffer = viewState.vertexBufferView;
		renderBlock->vertexLayout = m_vertexLayout;
		renderBlock->primitive = render::PrimitiveType::Triangles;
		renderBlock->offset = batch.offset;
		renderBlock->count = batch.count;

		renderBlock->programParams->beginParameters(renderContext);

		// Quads are already in world space.
		worldRenderPass.setProgramParameters(
			renderBlock->programParams,
			Transform::identity(),
			Transform::identity());

		renderBlock->programParams->setTextureParameter(s_handleBillboardAlbedo, batch.albedoTexture);
		renderBlock->programParams->setTextureParameter(s_handleBillboardNormals, batch.normalTexture);
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
	vertexElements.push_back(render::VertexElement(render::DataUsage::Normal, render::DtFloat3, offsetof(BillboardVertex, normal), 0));
	vertexElements.push_back(render::VertexElement(render::DataUsage::Tangent, render::DtFloat3, offsetof(BillboardVertex, tangent), 0));
	vertexElements.push_back(render::VertexElement(render::DataUsage::Binormal, render::DtFloat3, offsetof(BillboardVertex, binormal), 0));
	vertexElements.push_back(render::VertexElement(render::DataUsage::Custom, render::DtFloat2, offsetof(BillboardVertex, texCoord), 0));
	T_ASSERT_M(render::getVertexSize(vertexElements) == sizeof(BillboardVertex), L"Incorrect size of vertex");
	m_vertexLayout = renderSystem->createVertexLayout(vertexElements);

	// Dynamic buffer rotate on each unlock, keeping views of previous frames intact.
	m_vertexBuffer = renderSystem->createBuffer(render::BuVertex, c_maxBillboards * 4 * sizeof(BillboardVertex), true, T_FILE_LINE_W);
	if (!m_vertexBuffer)
		return false;

	m_indexBuffer = renderSystem->createBuffer(render::BuIndex, c_maxBillboards * 6 * sizeof(uint16_t), false, T_FILE_LINE_W);
	if (!m_indexBuffer)
		return false;

	uint16_t* index = static_cast< uint16_t* >(m_indexBuffer->lock());
	if (!index)
		return false;

	// Counter clockwise as seen from the front.
	for (uint32_t i = 0; i < c_maxBillboards; ++i)
	{
		const uint16_t base = (uint16_t)(i * 4);
		*index++ = base + 0;
		*index++ = base + 2;
		*index++ = base + 1;
		*index++ = base + 0;
		*index++ = base + 3;
		*index++ = base + 2;
	}

	m_indexBuffer->unlock();
	return true;
}

}
