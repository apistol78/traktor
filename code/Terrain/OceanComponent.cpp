/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Math/Float.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Heightfield/Heightfield.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Resource/IResourceManager.h"
#include "Terrain/OceanComponent.h"
#include "Terrain/OceanComponentData.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainComponent.h"
#include "World/Entity.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldRenderView.h"
#include "World/WorldSetupContext.h"

namespace traktor::terrain
{
	namespace
	{

const render::Handle s_handleTerrain_Heightfield(L"Terrain_Heightfield");
const render::Handle s_handleTerrain_WorldOrigin(L"Terrain_WorldOrigin");
const render::Handle s_handleTerrain_WorldExtent(L"Terrain_WorldExtent");
const render::Handle s_handleOcean_HaveTerrain(L"Ocean_HaveTerrain");
const render::Handle s_handleOcean_Eye(L"Ocean_Eye");
const render::Handle s_handleOcean_ShallowTint(L"Ocean_ShallowTint");
const render::Handle s_handleOcean_DeepColor(L"Ocean_DeepColor");
const render::Handle s_handleOcean_Opacity(L"Ocean_Opacity");
const render::Handle s_handleOcean_WaveTexture(L"Ocean_WaveTexture");
const render::Handle s_handleOcean_WaveTexture0(L"Ocean_WaveTexture0");
const render::Handle s_handleOcean_WaveTexture1(L"Ocean_WaveTexture1");
const render::Handle s_handleOcean_WaveTexture2(L"Ocean_WaveTexture2");
const render::Handle s_handleOcean_WaveTexture3(L"Ocean_WaveTexture3");
const render::Handle s_handleOcean_TileIndex(L"Ocean_TileIndex");
const render::Handle s_handleWorld_Time(L"World_Time");

#pragma pack(1)
struct OceanVertex
{
	float pos[2];
	float edge;
};
#pragma pack()

const uint32_t c_gridSize = 512;
const uint32_t c_gridInfSize = c_gridSize / 8;
const uint32_t c_gridCells = (c_gridSize - 1) * (c_gridSize - 1);

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.OceanComponent", OceanComponent, world::IEntityComponent)

OceanComponent::~OceanComponent()
{
	destroy();
}

bool OceanComponent::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const OceanComponentData& data)
{
	render::SimpleTextureCreateDesc stcd = {};
	stcd.width = 512;
	stcd.height = 512;
	stcd.mipCount = 1;
	stcd.format = render::TfR32F;
	stcd.shaderStorage = true;
	for (int32_t i = 0; i < sizeof_array(m_waveTextures); ++i)
	{
		m_waveTextures[i] = renderSystem->createSimpleTexture(stcd, T_FILE_LINE_W);
		if (!m_waveTextures[i])
			return false;
	}

	stcd.format = render::TfR32G32B32A32F;
	m_spectrumTexture = renderSystem->createSimpleTexture(stcd, T_FILE_LINE_W);
	m_evolvedSpectrumTexture = renderSystem->createSimpleTexture(stcd, T_FILE_LINE_W);

	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DataUsage::Position, render::DtFloat2, offsetof(OceanVertex, pos)));
	vertexElements.push_back(render::VertexElement(render::DataUsage::Custom, render::DtFloat1, offsetof(OceanVertex, edge)));
	m_vertexLayout = renderSystem->createVertexLayout(vertexElements);

	m_vertexBuffer = renderSystem->createBuffer(render::BuVertex, c_gridSize * c_gridSize * sizeof(OceanVertex), false);
	if (!m_vertexBuffer)
		return false;

	OceanVertex* vertex = reinterpret_cast< OceanVertex* >(m_vertexBuffer->lock());
	T_ASSERT(vertex);

	for (int32_t iz = 0; iz < c_gridSize; ++iz)
	{
		const float fz = float(iz) * 2.0f / c_gridSize - 1.0f;
		const float ez0 = clamp(1.0f - float(iz) / c_gridInfSize, 0.0f, 1.0f);
		const float ez1 = clamp(1.0f - float(c_gridSize - 1 - iz) / c_gridInfSize, 0.0f, 1.0f);

		for (int32_t ix = 0; ix < c_gridSize; ++ix)
		{
			const float fx = float(ix) * 2.0f / c_gridSize - 1.0f;
			const float ex0 = clamp(1.0f - float(ix) / c_gridInfSize, 0.0f, 1.0f);
			const float ex1 = clamp(1.0f - float(c_gridSize - 1 - ix) / c_gridInfSize, 0.0f, 1.0f);
			const float f = std::pow(max(max(ex0, ex1), max(ez0, ez1)), 8);

			vertex->pos[0] = fx;
			vertex->pos[1] = fz;
			vertex->edge = lerp(100.0f, 10000.0f, f);

			vertex++;
		}
	}

	m_vertexBuffer->unlock();

	m_indexBuffer = renderSystem->createBuffer(render::BuIndex, c_gridCells * 6 * sizeof(uint32_t), false);
	if (!m_indexBuffer)
		return false;

	uint32_t* index = reinterpret_cast< uint32_t* >(m_indexBuffer->lock());
	T_ASSERT(index);

	for (uint32_t iz = 0; iz < c_gridSize - 1; ++iz)
	{
		const uint32_t base = iz * c_gridSize;
		for (uint32_t ix = 0; ix < c_gridSize - 1; ++ix)
		{
			const uint32_t quad[] =
			{
				base + ix,
				base + ix + 1,
				base + ix + 1 + c_gridSize,
				base + ix + c_gridSize
			};

			*index++ = quad[0];
			*index++ = quad[1];
			*index++ = quad[2];

			*index++ = quad[2];
			*index++ = quad[3];
			*index++ = quad[0];
		}
	}

	m_indexBuffer->unlock();

	m_primitives.setIndexed(render::PrimitiveType::Triangles, 0, c_gridCells * 2, 0, c_gridSize * c_gridSize - 1);

	if (!resourceManager->bind(data.m_shaderWave, m_shaderWave))
		return false;
	if (!resourceManager->bind(data.m_shader, m_shader))
		return false;

	m_shallowTint = data.m_shallowTint;
	m_deepColor = data.m_deepColor;
	m_opacity = data.m_opacity;
	m_elevation = data.m_elevation;

	return true;
}

void OceanComponent::destroy()
{
	safeDestroy(m_vertexBuffer);
	safeDestroy(m_indexBuffer);
	for (int32_t i = 0; i < sizeof_array(m_waveTextures); ++i)
		safeDestroy(m_waveTextures[i]);
	m_shader.clear();
}

void OceanComponent::setOwner(world::Entity* owner)
{
	m_owner = owner;
}

void OceanComponent::setTransform(const Transform& transform)
{
}

Aabb3 OceanComponent::getBoundingBox() const
{
	return Aabb3();
}

void OceanComponent::update(const world::UpdateParams& update)
{
}

void OceanComponent::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView
)
{

	/* Do once begin */

	{
		Ref< render::RenderPass > rp = new render::RenderPass(L"Ocean compute spectrum");
		rp->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {
			auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"Ocean spectrum");

			const render::Shader::Permutation perm(render::getParameterHandle(L"Spectrum"));

			renderBlock->program = m_shaderWave->getProgram(perm).program;
			renderBlock->workSize[0] = 512;
			renderBlock->workSize[1] = 512;
			renderBlock->workSize[2] = 1;

			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->programParams->beginParameters(renderContext);
			renderBlock->programParams->setFloatParameter(s_handleWorld_Time, worldRenderView.getTime());
			renderBlock->programParams->setFloatParameter(s_handleOcean_TileIndex, 0);
			renderBlock->programParams->setImageViewParameter(s_handleOcean_WaveTexture, m_spectrumTexture, 0);
			renderBlock->programParams->endParameters(renderContext);

			renderContext->compute(renderBlock);
			renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Vertex, nullptr, 0);
		});
		context.getRenderGraph().addPass(rp);
	}

	{
		Ref< render::RenderPass > rp = new render::RenderPass(L"Ocean compute spectrum pack conjugate");
		rp->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {
			auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"Ocean spectrum pack conjugate");

			const render::Shader::Permutation perm(render::getParameterHandle(L"SpectrumPackConjugate"));

			renderBlock->program = m_shaderWave->getProgram(perm).program;
			renderBlock->workSize[0] = 512;
			renderBlock->workSize[1] = 512;
			renderBlock->workSize[2] = 1;

			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->programParams->beginParameters(renderContext);
			renderBlock->programParams->setFloatParameter(s_handleWorld_Time, worldRenderView.getTime());
			renderBlock->programParams->setFloatParameter(s_handleOcean_TileIndex, 0);
			renderBlock->programParams->setImageViewParameter(s_handleOcean_WaveTexture, m_spectrumTexture, 0);
			renderBlock->programParams->endParameters(renderContext);

			renderContext->compute(renderBlock);
			renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Vertex, nullptr, 0);
		});
		context.getRenderGraph().addPass(rp);
	}

	/* Do once end */

	{
		Ref< render::RenderPass > rp = new render::RenderPass(L"Ocean compute spectrum evolve");
		rp->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {
			auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"Ocean spectrum evolve");

			const render::Shader::Permutation perm(render::getParameterHandle(L"SpectrumEvolve"));

			renderBlock->program = m_shaderWave->getProgram(perm).program;
			renderBlock->workSize[0] = 512;
			renderBlock->workSize[1] = 512;
			renderBlock->workSize[2] = 1;

			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->programParams->beginParameters(renderContext);
			renderBlock->programParams->setFloatParameter(s_handleWorld_Time, worldRenderView.getTime());
			renderBlock->programParams->setFloatParameter(s_handleOcean_TileIndex, 0);
			renderBlock->programParams->setImageViewParameter(s_handleOcean_WaveTexture, m_spectrumTexture, 0);
			renderBlock->programParams->setImageViewParameter(s_handleOcean_WaveTexture0, m_evolvedSpectrumTexture, 0);
			renderBlock->programParams->endParameters(renderContext);

			renderContext->compute(renderBlock);
			renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Vertex, nullptr, 0);
		});
		context.getRenderGraph().addPass(rp);
	}

	{
		Ref< render::RenderPass > rp = new render::RenderPass(L"Ocean compute inverse FFT X");
		rp->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {
			auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"Ocean inverse FFT X");

			const render::Shader::Permutation perm(render::getParameterHandle(L"InverseFFT_X"));

			renderBlock->program = m_shaderWave->getProgram(perm).program;
			renderBlock->workSize[0] = 512;
			renderBlock->workSize[1] = 512;
			renderBlock->workSize[2] = 1;

			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->programParams->beginParameters(renderContext);
			renderBlock->programParams->setFloatParameter(s_handleWorld_Time, worldRenderView.getTime());
			renderBlock->programParams->setFloatParameter(s_handleOcean_TileIndex, 0);
			renderBlock->programParams->setImageViewParameter(s_handleOcean_WaveTexture, m_evolvedSpectrumTexture, 0);
			renderBlock->programParams->setImageViewParameter(s_handleOcean_WaveTexture0, m_evolvedSpectrumTexture, 0);
			renderBlock->programParams->endParameters(renderContext);

			renderContext->compute(renderBlock);
			renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Vertex, nullptr, 0);
		});
		context.getRenderGraph().addPass(rp);
	}

	{
		Ref< render::RenderPass > rp = new render::RenderPass(L"Ocean compute inverse FFT Y");
		rp->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {
			auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"Ocean inverse FFT Y");

			const render::Shader::Permutation perm(render::getParameterHandle(L"InverseFFT_Y"));

			renderBlock->program = m_shaderWave->getProgram(perm).program;
			renderBlock->workSize[0] = 512;
			renderBlock->workSize[1] = 512;
			renderBlock->workSize[2] = 1;

			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->programParams->beginParameters(renderContext);
			renderBlock->programParams->setFloatParameter(s_handleWorld_Time, worldRenderView.getTime());
			renderBlock->programParams->setFloatParameter(s_handleOcean_TileIndex, 0);
			renderBlock->programParams->setImageViewParameter(s_handleOcean_WaveTexture, m_evolvedSpectrumTexture, 0);
			renderBlock->programParams->setImageViewParameter(s_handleOcean_WaveTexture0, m_evolvedSpectrumTexture, 0);
			renderBlock->programParams->endParameters(renderContext);

			renderContext->compute(renderBlock);
			renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Vertex, nullptr, 0);
		});
		context.getRenderGraph().addPass(rp);
	}

	for (int32_t i = 0; i < sizeof_array(m_waveTextures); ++i)
	{
		Ref< render::RenderPass > rp = new render::RenderPass(L"Ocean compute waves");
		rp->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {
			auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(str(L"Ocean wave %d", i));
			renderBlock->program = m_shaderWave->getProgram().program;
			renderBlock->workSize[0] = 512;
			renderBlock->workSize[1] = 512;
			renderBlock->workSize[2] = 1;

			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->programParams->beginParameters(renderContext);
			renderBlock->programParams->setFloatParameter(s_handleWorld_Time, worldRenderView.getTime());
			renderBlock->programParams->setFloatParameter(s_handleOcean_TileIndex, i);
			renderBlock->programParams->setImageViewParameter(s_handleOcean_WaveTexture, m_waveTextures[i], 0);
			renderBlock->programParams->endParameters(renderContext);

			renderContext->compute(renderBlock);
			renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Vertex, nullptr, 0);
		});
		context.getRenderGraph().addPass(rp);
	}
}

void OceanComponent::build(
	render::RenderContext* renderContext,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	bool reflectionEnable
)
{
	if (!m_owner || worldRenderView.getSnapshot())
		return;

	bool haveTerrain = false;

	// Get terrain from owner.
	auto terrainComponent = m_owner->getComponent< TerrainComponent >();
	if (terrainComponent)
	{
		const auto& terrain = terrainComponent->getTerrain();
		haveTerrain = (terrain && terrain->getHeightfield() && terrain->getHeightMap());
	}

	const Transform transform = m_owner->getTransform() * Transform(Vector4(0.0f, m_elevation, 0.0f, 0.0f));
	const Matrix44& view = worldRenderView.getView();
	const Matrix44 viewInv = view.inverse();
	const Vector4 eye = viewInv.translation().xyz1();

	// Render ocean geometry.
	auto perm = worldRenderPass.getPermutation(m_shader);
	m_shader->setCombination(s_handleOcean_HaveTerrain, haveTerrain, perm);
	auto sp = m_shader->getProgram(perm);
	if (!sp)
		return;

	auto renderBlock = renderContext->allocNamed< render::SimpleRenderBlock >(L"Ocean");
	renderBlock->distance = std::numeric_limits< float >::max();
	renderBlock->program = sp.program;
	renderBlock->indexBuffer = m_indexBuffer->getBufferView();
	renderBlock->indexType = render::IndexType::UInt32;
	renderBlock->vertexBuffer = m_vertexBuffer->getBufferView();
	renderBlock->vertexLayout = m_vertexLayout;
	renderBlock->primitives = m_primitives;

	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->programParams->beginParameters(renderContext);
	renderBlock->programParams->setFloatParameter(s_handleOcean_Opacity, m_opacity);
	renderBlock->programParams->setVectorParameter(s_handleOcean_Eye, eye);
	renderBlock->programParams->setVectorParameter(s_handleOcean_ShallowTint, m_shallowTint);
	renderBlock->programParams->setVectorParameter(s_handleOcean_DeepColor, m_deepColor);
	renderBlock->programParams->setTextureParameter(s_handleOcean_WaveTexture0, m_waveTextures[0]);
	renderBlock->programParams->setTextureParameter(s_handleOcean_WaveTexture1, m_waveTextures[1]);
	renderBlock->programParams->setTextureParameter(s_handleOcean_WaveTexture2, m_waveTextures[2]);
	renderBlock->programParams->setTextureParameter(s_handleOcean_WaveTexture3, m_waveTextures[3]);

	if (haveTerrain)
	{
		const auto& terrain = terrainComponent->getTerrain();

		const Vector4& worldExtent = terrain->getHeightfield()->getWorldExtent();
		const Vector4 worldOrigin = -worldExtent * 0.5_simd;

		renderBlock->programParams->setVectorParameter(s_handleTerrain_WorldOrigin, worldOrigin);
		renderBlock->programParams->setVectorParameter(s_handleTerrain_WorldExtent, worldExtent);
		renderBlock->programParams->setTextureParameter(s_handleTerrain_Heightfield, terrain->getHeightMap());
	}

	worldRenderPass.setProgramParameters(
		renderBlock->programParams,
		transform,
		transform
	);

	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(sp.priority, renderBlock);
}

}
