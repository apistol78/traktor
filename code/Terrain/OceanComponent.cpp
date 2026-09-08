/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/OceanComponent.h"

#include "Core/Math/Float.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Heightfield/Heightfield.h"
#include "Render/Buffer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/VertexElement.h"
#include "Resource/IResourceManager.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainComponent.h"
#include "World/Entity.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/WorldSetupContext.h"

#include <algorithm>
#include <cstring>
#include <limits>

namespace traktor::terrain
{
namespace
{

// Spectrum parameters
const render::Handle s_handleOcean_SpectrumScale(L"Ocean_SpectrumScale");
const render::Handle s_handleOcean_SpectrumAngle(L"Ocean_SpectrumAngle");
const render::Handle s_handleOcean_SpectrumSpreadBlend(L"Ocean_SpectrumSpreadBlend");
const render::Handle s_handleOcean_SpectrumSwell(L"Ocean_SpectrumSwell");
const render::Handle s_handleOcean_SpectrumAlpha(L"Ocean_SpectrumAlpha");
const render::Handle s_handleOcean_SpectrumPeakOmega(L"Ocean_SpectrumPeakOmega");
const render::Handle s_handleOcean_SpectrumGamma(L"Ocean_SpectrumGamma");
const render::Handle s_handleOcean_SpectrumShortWavesFade(L"Ocean_SpectrumShortWavesFade");

// Ocean
const render::Handle s_handleTerrain_Heightfield(L"Terrain_Heightfield");
const render::Handle s_handleTerrain_WorldOrigin(L"Terrain_WorldOrigin");
const render::Handle s_handleTerrain_WorldExtent(L"Terrain_WorldExtent");
const render::Handle s_handleOcean_HaveTerrain(L"Ocean_HaveTerrain");
const render::Handle s_handleOcean_Eye(L"Ocean_Eye");
const render::Handle s_handleOcean_LastEye(L"Ocean_LastEye");
const render::Handle s_handleOcean_ShallowTint(L"Ocean_ShallowTint");
const render::Handle s_handleOcean_DeepColor(L"Ocean_DeepColor");
const render::Handle s_handleOcean_Opacity(L"Ocean_Opacity");
const render::Handle s_handleOcean_WaveTexture(L"Ocean_WaveTexture");
const render::Handle s_handleOcean_WaveTexture0(L"Ocean_WaveTexture0");
const render::Handle s_handleOcean_WaveTexture1(L"Ocean_WaveTexture1");
const render::Handle s_handleOcean_WaveTexture2(L"Ocean_WaveTexture2");
const render::Handle s_handleOcean_WaveTexture3(L"Ocean_WaveTexture3");
const render::Handle s_handleOcean_WaveOutput0(L"Ocean_WaveOutput0");
const render::Handle s_handleOcean_WaveOutput1(L"Ocean_WaveOutput1");
const render::Handle s_handleOcean_WaveOutput2(L"Ocean_WaveOutput2");
const render::Handle s_handleOcean_TileIndex(L"Ocean_TileIndex");
const render::Handle s_handleWorld_Time(L"World_Time");

// Tile culling; shares cull shader and patch data layout with terrain.
const resource::Id< render::Shader > c_shaderCull(L"{8BA73DD8-0FD9-4C15-A772-EACC14014AEC}");
const render::Handle s_handleTerrain_ViewProjection(L"Terrain_ViewProjection");
const render::Handle s_handleTerrain_TargetSize(L"Terrain_TargetSize");
const render::Handle s_handleTerrain_DrawBuffer(L"Terrain_DrawBuffer");
const render::Handle s_handleTerrain_CulledDrawBuffer(L"Terrain_CulledDrawBuffer");
const render::Handle s_handleTerrain_PatchData(L"Terrain_PatchData");

#pragma pack(1)

struct OceanVertex
{
	float pos[2];
};

#pragma pack()

const uint32_t c_spectrumSize = 1024;

//! Maximum vertical displacement of waves; used to build tile bounding boxes.
const float c_maxWaveHeight = 10.0f;

//! Per tile data, must match layout of "Terrain_PatchData" structure.
struct DrawData
{
	float patchOrigin[4];
	float surfaceOffset[4];
	float patchBoundingBoxMn[4];
	float patchBoundingBoxMx[4];
};

enum StitchEdge
{
	SeLeft = 1,		// -X
	SeRight = 2,	// +X
	SeNear = 4,		// -Z
	SeFar = 8		// +Z
};

/*! Append triangle with consistent winding, counter clockwise seen from above (+Y). */
void addTriangle(AlignedVector< uint32_t >& indices, uint32_t gridSize, uint32_t ax, uint32_t az, uint32_t bx, uint32_t bz, uint32_t cx, uint32_t cz)
{
	const int32_t area = (int32_t(bx) - int32_t(ax)) * (int32_t(cz) - int32_t(az)) - (int32_t(cx) - int32_t(ax)) * (int32_t(bz) - int32_t(az));
	if (area == 0)
		return;
	if (area < 0)
	{
		std::swap(bx, cx);
		std::swap(bz, cz);
	}
	indices.push_back(ax + az * gridSize);
	indices.push_back(bx + bz * gridSize);
	indices.push_back(cx + cz * gridSize);
}

void addQuad(AlignedVector< uint32_t >& indices, uint32_t gridSize, uint32_t x, uint32_t z)
{
	addTriangle(indices, gridSize, x, z, x + 1, z, x + 1, z + 1);
	addTriangle(indices, gridSize, x + 1, z + 1, x, z + 1, x, z);
}

/*! Generate tile indices, cells is the number of cells along an edge (must be even and >= 4).
 *
 * Edges flagged in stitchMask are generated with half the vertex density so
 * they match a neighbour tile of the next coarser level of detail without cracks.
 */
void generateTileIndices(AlignedVector< uint32_t >& indices, uint32_t cells, uint32_t stitchMask)
{
	const uint32_t gridSize = cells + 1;
	const uint32_t last = cells - 1;

	const bool left = (stitchMask & SeLeft) != 0;
	const bool right = (stitchMask & SeRight) != 0;
	const bool stitchNear = (stitchMask & SeNear) != 0;
	const bool stitchFar = (stitchMask & SeFar) != 0;

	// Interior and edge strips; corners are 2x2 blocks handled separately.
	for (uint32_t cz = 0; cz < cells; ++cz)
	{
		for (uint32_t cx = 0; cx < cells; ++cx)
		{
			const bool cornerX = (cx < 2 || cx >= cells - 2);
			const bool cornerZ = (cz < 2 || cz >= cells - 2);
			if (cornerX && cornerZ)
				continue;

			if (cx == 0 && left)
			{
				// Fan from even edge vertex covering two cells.
				if ((cz & 1) == 0)
				{
					addTriangle(indices, gridSize, 0, cz, 1, cz, 1, cz + 1);
					addTriangle(indices, gridSize, 0, cz, 1, cz + 1, 1, cz + 2);
					addTriangle(indices, gridSize, 0, cz, 1, cz + 2, 0, cz + 2);
				}
			}
			else if (cx == last && right)
			{
				if ((cz & 1) == 0)
				{
					addTriangle(indices, gridSize, cells, cz, last, cz, last, cz + 1);
					addTriangle(indices, gridSize, cells, cz, last, cz + 1, last, cz + 2);
					addTriangle(indices, gridSize, cells, cz, last, cz + 2, cells, cz + 2);
				}
			}
			else if (cz == 0 && stitchNear)
			{
				if ((cx & 1) == 0)
				{
					addTriangle(indices, gridSize, cx, 0, cx, 1, cx + 1, 1);
					addTriangle(indices, gridSize, cx, 0, cx + 1, 1, cx + 2, 1);
					addTriangle(indices, gridSize, cx, 0, cx + 2, 1, cx + 2, 0);
				}
			}
			else if (cz == last && stitchFar)
			{
				if ((cx & 1) == 0)
				{
					addTriangle(indices, gridSize, cx, cells, cx, last, cx + 1, last);
					addTriangle(indices, gridSize, cx, cells, cx + 1, last, cx + 2, last);
					addTriangle(indices, gridSize, cx, cells, cx + 2, last, cx + 2, cells);
				}
			}
			else
				addQuad(indices, gridSize, cx, cz);
		}
	}

	// Corner blocks; (ox, oz) is the corner vertex, (dx, dz) direction into the tile.
	const struct
	{
		uint32_t ox, oz;
		int32_t dx, dz;
		bool edgeX, edgeZ;	// Stitched along the X-running edge (near/far) and Z-running edge (left/right).
	}
	corners[] = {
		{ 0, 0, 1, 1, stitchNear, left },
		{ cells, 0, -1, 1, stitchNear, right },
		{ 0, cells, 1, -1, stitchFar, left },
		{ cells, cells, -1, -1, stitchFar, right }
	};

	for (const auto& c : corners)
	{
		// Local corner coordinates (0..2) to grid coordinates.
		const auto X = [&](int32_t lx) { return uint32_t(int32_t(c.ox) + lx * c.dx); };
		const auto Z = [&](int32_t lz) { return uint32_t(int32_t(c.oz) + lz * c.dz); };
		const auto tri = [&](int32_t ax, int32_t az, int32_t bx, int32_t bz, int32_t cx, int32_t cz) {
			addTriangle(indices, gridSize, X(ax), Z(az), X(bx), Z(bz), X(cx), Z(cz));
		};

		if (c.edgeX && c.edgeZ)
		{
			// Both edges stitched; drop odd vertices on both edges.
			tri(0, 0, 2, 0, 1, 1);
			tri(2, 0, 2, 1, 1, 1);
			tri(0, 0, 1, 1, 0, 2);
			tri(0, 2, 1, 1, 1, 2);
			tri(1, 1, 2, 1, 2, 2);
			tri(1, 1, 2, 2, 1, 2);
		}
		else if (c.edgeZ)
		{
			// Edge running along Z stitched; fan from corner covering two cells, rest quads.
			tri(0, 0, 1, 0, 1, 1);
			tri(0, 0, 1, 1, 1, 2);
			tri(0, 0, 1, 2, 0, 2);
			tri(1, 0, 2, 0, 2, 1); tri(2, 1, 1, 1, 1, 0);
			tri(1, 1, 2, 1, 2, 2); tri(2, 2, 1, 2, 1, 1);
		}
		else if (c.edgeX)
		{
			tri(0, 0, 0, 1, 1, 1);
			tri(0, 0, 1, 1, 2, 1);
			tri(0, 0, 2, 1, 2, 0);
			tri(0, 1, 1, 1, 1, 2); tri(1, 2, 0, 2, 0, 1);
			tri(1, 1, 2, 1, 2, 2); tri(2, 2, 1, 2, 1, 1);
		}
		else
		{
			tri(0, 0, 1, 0, 1, 1); tri(1, 1, 0, 1, 0, 0);
			tri(1, 0, 2, 0, 2, 1); tri(2, 1, 1, 1, 1, 0);
			tri(0, 1, 1, 1, 1, 2); tri(1, 2, 0, 2, 0, 1);
			tri(1, 1, 2, 1, 2, 2); tri(2, 2, 1, 2, 1, 1);
		}
	}
}

void setSpectrumParameters(const OceanComponentData::Spectrum& spectrum, render::ProgramParameters* params)
{
	params->setFloatParameter(s_handleOcean_SpectrumScale, spectrum.scale);
	params->setFloatParameter(s_handleOcean_SpectrumAngle, spectrum.angle);
	params->setFloatParameter(s_handleOcean_SpectrumSpreadBlend, spectrum.spreadBlend);
	params->setFloatParameter(s_handleOcean_SpectrumSwell, spectrum.swell);
	params->setFloatParameter(s_handleOcean_SpectrumAlpha, spectrum.alpha);
	params->setFloatParameter(s_handleOcean_SpectrumPeakOmega, spectrum.peakOmega);
	params->setFloatParameter(s_handleOcean_SpectrumGamma, spectrum.gamma);
	params->setFloatParameter(s_handleOcean_SpectrumShortWavesFade, spectrum.shortWavesFade);
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.OceanComponent", OceanComponent, world::IEntityComponent)

OceanComponent::~OceanComponent()
{
	destroy();
}

bool OceanComponent::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const OceanComponentData& data)
{
	render::SimpleTextureCreateDesc stcd = {};
	stcd.width = c_spectrumSize;
	stcd.height = c_spectrumSize;
	stcd.mipCount = 1;
	stcd.format = render::TfR16G16B16A16F;
	stcd.shaderStorage = true;
	m_spectrumTexture = renderSystem->createSimpleTexture(stcd, T_FILE_LINE_W);
	m_evolvedSpectrumTextures[0] = renderSystem->createSimpleTexture(stcd, T_FILE_LINE_W);
	m_evolvedSpectrumTextures[1] = renderSystem->createSimpleTexture(stcd, T_FILE_LINE_W);
	m_evolvedSpectrumTextures[2] = renderSystem->createSimpleTexture(stcd, T_FILE_LINE_W);
	m_evolvedSpectrumTextures[3] = renderSystem->createSimpleTexture(stcd, T_FILE_LINE_W);
	m_foamTexture = renderSystem->createSimpleTexture(stcd, T_FILE_LINE_W);

	m_renderSystem = renderSystem;
	m_gridCells = 32 << clamp(data.m_quality, 0, 3);
	m_tileSize = max(data.m_tileSize, 1.0f);
	m_lodCount = clamp(data.m_lodCount, 1, 12);

	// Create tile vertices; a unit grid which is scaled and offset per tile in vertex shader.
	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DataUsage::Position, render::DtFloat2, offsetof(OceanVertex, pos)));
	m_vertexLayout = renderSystem->createVertexLayout(vertexElements);

	const uint32_t gridSize = m_gridCells + 1;

	m_vertexBuffer = renderSystem->createBuffer(render::BuVertex, gridSize * gridSize * sizeof(OceanVertex), false, T_FILE_LINE_W);
	if (!m_vertexBuffer)
		return false;

	OceanVertex* vertex = reinterpret_cast< OceanVertex* >(m_vertexBuffer->lock());
	T_ASSERT(vertex);

	for (uint32_t iz = 0; iz < gridSize; ++iz)
	{
		const float fz = float(iz) / float(m_gridCells);
		for (uint32_t ix = 0; ix < gridSize; ++ix)
		{
			const float fx = float(ix) / float(m_gridCells);
			vertex->pos[0] = fx;
			vertex->pos[1] = fz;
			vertex++;
		}
	}

	m_vertexBuffer->unlock();

	// Create tile indices, one set for each combination of stitched edges.
	AlignedVector< uint32_t > indices;
	for (uint32_t stitchMask = 0; stitchMask < 16; ++stitchMask)
	{
		const uint32_t offset = (uint32_t)indices.size();
		generateTileIndices(indices, m_gridCells, stitchMask);
		m_primitives[stitchMask] = render::Primitives::setIndexed(
			render::PrimitiveType::Triangles,
			offset,
			(uint32_t)(indices.size() - offset) / 3);
	}

	m_indexBuffer = renderSystem->createBuffer(render::BuIndex, (uint32_t)indices.size() * sizeof(uint32_t), false, T_FILE_LINE_W);
	if (!m_indexBuffer)
		return false;

	uint32_t* index = reinterpret_cast< uint32_t* >(m_indexBuffer->lock());
	T_ASSERT(index);
	std::memcpy(index, indices.c_ptr(), indices.size() * sizeof(uint32_t));
	m_indexBuffer->unlock();

	// Create per tile draw and data buffers; grown on demand.
	if (!createTileBuffers(64 * m_lodCount))
		return false;

	if (!resourceManager->bind(c_shaderCull, m_shaderCull))
		return false;
	if (!resourceManager->bind(data.m_shaderWave, m_shaderWave))
		return false;
	if (!resourceManager->bind(data.m_shader, m_shader))
		return false;

	m_spectrum = data.m_spectrum;
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
	safeDestroy(m_drawBuffer);
	safeDestroy(m_culledDrawBuffer);
	safeDestroy(m_dataBuffer);
	safeDestroy(m_spectrumTexture);
	safeDestroy(m_evolvedSpectrumTextures[0]);
	safeDestroy(m_evolvedSpectrumTextures[1]);
	safeDestroy(m_evolvedSpectrumTextures[2]);
	safeDestroy(m_evolvedSpectrumTextures[3]);
	safeDestroy(m_foamTexture);
	m_shaderCull.clear();
	m_shaderWave.clear();
	m_shader.clear();
	m_renderSystem = nullptr;
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
	const world::WorldRenderView& worldRenderView)
{
	// Determine which tiles are visible and update draw buffers.
	updateTiles(worldRenderView);

	// Re-generate spectrum if shader has been modified.
	if (m_shaderWave.changed())
	{
		m_spectrumDirty = true;
		m_shaderWave.consume();
	}

	// Compute ocean wave spectrum.
	if (m_spectrumDirty)
	{
		{
			Ref< render::RenderPass > rp = new render::RenderPass(L"Ocean compute spectrum");
			rp->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {
				auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"Ocean spectrum");

				const render::Shader::Permutation perm(render::getParameterHandle(L"Ocean_Spectrum"));

				renderBlock->program = m_shaderWave->getProgram(perm).program;
				renderBlock->workSize[0] = c_spectrumSize;
				renderBlock->workSize[1] = c_spectrumSize;
				renderBlock->workSize[2] = 1;

				renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
				renderBlock->programParams->beginParameters(renderContext);
				renderBlock->programParams->setFloatParameter(s_handleWorld_Time, worldRenderView.getTime());
				renderBlock->programParams->setFloatParameter(s_handleOcean_TileIndex, 0);
				renderBlock->programParams->setImageViewParameter(s_handleOcean_WaveTexture, m_spectrumTexture, 0);

				setSpectrumParameters(m_spectrum, renderBlock->programParams);

				renderBlock->programParams->endParameters(renderContext);

				renderContext->compute(renderBlock);
				renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Compute, nullptr, 0);
			});
			context.getRenderGraph().addPass(rp);
		}

		{
			Ref< render::RenderPass > rp = new render::RenderPass(L"Ocean compute spectrum pack conjugate");
			rp->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {
				auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"Ocean spectrum pack conjugate");

				const render::Shader::Permutation perm(render::getParameterHandle(L"Ocean_SpectrumPackConjugate"));

				renderBlock->program = m_shaderWave->getProgram(perm).program;
				renderBlock->workSize[0] = c_spectrumSize;
				renderBlock->workSize[1] = c_spectrumSize;
				renderBlock->workSize[2] = 1;

				renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
				renderBlock->programParams->beginParameters(renderContext);
				renderBlock->programParams->setFloatParameter(s_handleWorld_Time, worldRenderView.getTime());
				renderBlock->programParams->setFloatParameter(s_handleOcean_TileIndex, 0);
				renderBlock->programParams->setImageViewParameter(s_handleOcean_WaveTexture, m_spectrumTexture, 0);
				renderBlock->programParams->endParameters(renderContext);

				renderContext->compute(renderBlock);
				renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Compute, nullptr, 0);
			});
			context.getRenderGraph().addPass(rp);
		}

		m_spectrumDirty = false;
	}

	// Swap textures so we have last and current.
	{
		std::swap(m_evolvedSpectrumTextures[0], m_evolvedSpectrumTextures[2]);
		std::swap(m_evolvedSpectrumTextures[1], m_evolvedSpectrumTextures[3]);
	}

	render::RGDependency dependency = context.getRenderGraph().addDependency();

	// Evolve spectrum over time.
	{
		Ref< render::RenderPass > rp = new render::RenderPass(L"Ocean compute spectrum evolve");
		rp->setOutput(dependency);
		rp->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {
			auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"Ocean spectrum evolve");

			const render::Shader::Permutation perm(render::getParameterHandle(L"Ocean_SpectrumEvolve"));

			renderBlock->program = m_shaderWave->getProgram(perm).program;
			renderBlock->workSize[0] = c_spectrumSize;
			renderBlock->workSize[1] = c_spectrumSize;
			renderBlock->workSize[2] = 1;

			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->programParams->beginParameters(renderContext);
			renderBlock->programParams->setFloatParameter(s_handleWorld_Time, worldRenderView.getTime());
			renderBlock->programParams->setFloatParameter(s_handleOcean_TileIndex, 0);
			renderBlock->programParams->setImageViewParameter(s_handleOcean_WaveTexture, m_spectrumTexture, 0);
			renderBlock->programParams->setImageViewParameter(s_handleOcean_WaveOutput0, m_evolvedSpectrumTextures[0], 0);
			renderBlock->programParams->setImageViewParameter(s_handleOcean_WaveOutput1, m_evolvedSpectrumTextures[1], 0);
			renderBlock->programParams->endParameters(renderContext);

			renderContext->compute(renderBlock);
			renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Compute, nullptr, 0);
		});
		context.getRenderGraph().addPass(rp);
	}

	// Compute inverse FFT of spectrums to get time domain heights.
	for (int32_t i = 0; i < 2; ++i)
	{
		const render::RGDependency d1 = context.getRenderGraph().addDependency();
		const render::RGDependency d2 = context.getRenderGraph().addDependency();

		{
			Ref< render::RenderPass > rp = new render::RenderPass(L"Ocean compute inverse FFT X");
			rp->setOutput(d1);
			rp->addInput(dependency);
			rp->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {
				auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"Ocean inverse FFT X");

				const render::Shader::Permutation perm(render::getParameterHandle(L"Ocean_InverseFFT_X"));

				renderBlock->program = m_shaderWave->getProgram(perm).program;
				renderBlock->workSize[0] = c_spectrumSize;
				renderBlock->workSize[1] = c_spectrumSize;
				renderBlock->workSize[2] = 1;

				renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
				renderBlock->programParams->beginParameters(renderContext);
				renderBlock->programParams->setFloatParameter(s_handleWorld_Time, worldRenderView.getTime());
				renderBlock->programParams->setFloatParameter(s_handleOcean_TileIndex, 0);
				renderBlock->programParams->setImageViewParameter(s_handleOcean_WaveTexture, m_evolvedSpectrumTextures[i], 0);
				renderBlock->programParams->setImageViewParameter(s_handleOcean_WaveOutput0, m_evolvedSpectrumTextures[i], 0);
				renderBlock->programParams->endParameters(renderContext);

				renderContext->compute(renderBlock);
				renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Compute, nullptr, 0);
			});
			context.getRenderGraph().addPass(rp);
		}

		{
			Ref< render::RenderPass > rp = new render::RenderPass(L"Ocean compute inverse FFT Y");
			rp->setOutput(d2);
			rp->addInput(d1);
			rp->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {
				auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"Ocean inverse FFT Y");

				const render::Shader::Permutation perm(render::getParameterHandle(L"Ocean_InverseFFT_Y"));

				renderBlock->program = m_shaderWave->getProgram(perm).program;
				renderBlock->workSize[0] = c_spectrumSize;
				renderBlock->workSize[1] = c_spectrumSize;
				renderBlock->workSize[2] = 1;

				renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
				renderBlock->programParams->beginParameters(renderContext);
				renderBlock->programParams->setFloatParameter(s_handleWorld_Time, worldRenderView.getTime());
				renderBlock->programParams->setFloatParameter(s_handleOcean_TileIndex, 0);
				renderBlock->programParams->setImageViewParameter(s_handleOcean_WaveTexture, m_evolvedSpectrumTextures[i], 0);
				renderBlock->programParams->setImageViewParameter(s_handleOcean_WaveOutput0, m_evolvedSpectrumTextures[i], 0);
				renderBlock->programParams->endParameters(renderContext);

				renderContext->compute(renderBlock);
				renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Compute, nullptr, 0);
			});
			context.getRenderGraph().addPass(rp);
		}

		dependency = d2;
	}

	const render::RGDependency computeDependency = context.getRenderGraph().addDependency();

	{
		Ref< render::RenderPass > rp = new render::RenderPass(L"Ocean compute generate");
		rp->setOutput(computeDependency);
		rp->addInput(dependency);
		rp->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {
			auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"Ocean generate");

			const render::Shader::Permutation perm(render::getParameterHandle(L"Ocean_Generate"));

			renderBlock->program = m_shaderWave->getProgram(perm).program;
			renderBlock->workSize[0] = c_spectrumSize;
			renderBlock->workSize[1] = c_spectrumSize;
			renderBlock->workSize[2] = 1;

			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->programParams->beginParameters(renderContext);
			renderBlock->programParams->setFloatParameter(s_handleWorld_Time, worldRenderView.getTime());
			renderBlock->programParams->setFloatParameter(s_handleOcean_TileIndex, 0);
			renderBlock->programParams->setImageViewParameter(s_handleOcean_WaveOutput0, m_evolvedSpectrumTextures[0], 0);
			renderBlock->programParams->setImageViewParameter(s_handleOcean_WaveOutput1, m_evolvedSpectrumTextures[1], 0);
			renderBlock->programParams->setImageViewParameter(s_handleOcean_WaveOutput2, m_foamTexture, 0);
			renderBlock->programParams->endParameters(renderContext);

			renderContext->compute(renderBlock);
			renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Vertex, nullptr, 0);
		});
		context.getRenderGraph().addPass(rp);
	}

	// All passes drawing entities, including the velocity pass, must wait for the wave textures.
	context.addSetupAttachment(computeDependency);
}

void OceanComponent::build(
	render::RenderContext* renderContext,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass)
{
	if (!m_owner)
		return;

	View& view = m_view[worldRenderView.getIndex()];
	if (view.tiles.empty())
		return;

	const render::handle_t technique = worldRenderPass.getTechnique();
	const bool snapshot = worldRenderView.getSnapshot();
	const bool writeVelocity = (technique == world::ShaderTechnique::VelocityWrite);

	// Cull tiles against HiZ; only available when rendering GBuffer.
	if (technique == world::ShaderTechnique::DeferredGBufferWrite)
	{
		if (snapshot || !m_shaderCull)
			return;

		render::IProgram* cullProgram = m_shaderCull->getProgram().program;
		if (!cullProgram)
			return;

		const Vector2 viewSize = worldRenderView.getViewSize();

		auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"Ocean cull");
		renderBlock->program = cullProgram;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->programParams->beginParameters(renderContext);
		worldRenderPass.setProgramParameters(renderBlock->programParams);
		renderBlock->programParams->setVectorParameter(s_handleTerrain_TargetSize, Vector4(viewSize.x, viewSize.y, 0.0f, 0.0f));
		renderBlock->programParams->setMatrixParameter(s_handleTerrain_ViewProjection, worldRenderView.getProjection() * worldRenderView.getView());
		renderBlock->programParams->setBufferViewParameter(s_handleTerrain_DrawBuffer, m_drawBuffer->getBufferView());
		renderBlock->programParams->setBufferViewParameter(s_handleTerrain_CulledDrawBuffer, m_culledDrawBuffer->getBufferView());
		renderBlock->programParams->setBufferViewParameter(s_handleTerrain_PatchData, m_dataBuffer->getBufferView());
		renderBlock->programParams->endParameters(renderContext);
		renderBlock->workSize[0] = (int32_t)view.tiles.size();

		renderContext->compute(renderBlock);
		renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Indirect, nullptr, 0);

		view.culled = true;
		return;
	}

	bool haveTerrain = false;

	// Get terrain from owner.
	auto terrainComponent = m_owner->getComponent< TerrainComponent >();
	if (terrainComponent)
	{
		const auto& terrain = terrainComponent->getTerrain();
		haveTerrain = (terrain && terrain->getHeightfield() && terrain->getHeightMap());
	}

	const Transform transform = m_owner->getTransform() * Transform(Vector4(0.0f, m_elevation, 0.0f, 0.0f));

	// Render ocean tiles using indirect draw.
	auto perm = worldRenderPass.getPermutation(m_shader);
	m_shader->setCombination(s_handleOcean_HaveTerrain, haveTerrain, perm);
	const auto sp = m_shader->getProgram(perm);
	if (!sp)
		return;

	auto renderBlock = renderContext->allocNamed< render::IndirectRenderBlock >(L"Ocean");
	renderBlock->distance = std::numeric_limits< float >::max();
	renderBlock->program = sp.program;
	renderBlock->indexBuffer = m_indexBuffer->getBufferView();
	renderBlock->indexType = render::IndexType::UInt32;
	renderBlock->vertexBuffer = m_vertexBuffer->getBufferView();
	renderBlock->vertexLayout = m_vertexLayout;
	renderBlock->primitive = render::PrimitiveType::Triangles;
	renderBlock->drawBuffer = view.culled ? m_culledDrawBuffer->getBufferView() : m_drawBuffer->getBufferView();
	renderBlock->drawCount = (uint32_t)view.tiles.size();

	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->programParams->beginParameters(renderContext);
	renderBlock->programParams->setFloatParameter(s_handleOcean_Opacity, m_opacity);
	renderBlock->programParams->setVectorParameter(s_handleOcean_ShallowTint, m_shallowTint.linear());
	renderBlock->programParams->setVectorParameter(s_handleOcean_DeepColor, m_deepColor.linear());
	renderBlock->programParams->setBufferViewParameter(s_handleTerrain_PatchData, m_dataBuffer->getBufferView());

	if (!writeVelocity)
	{
		renderBlock->programParams->setTextureParameter(s_handleOcean_WaveTexture0, m_evolvedSpectrumTextures[0]);
		renderBlock->programParams->setTextureParameter(s_handleOcean_WaveTexture1, m_evolvedSpectrumTextures[1]);
		renderBlock->programParams->setTextureParameter(s_handleOcean_WaveTexture2, m_foamTexture);
	}
	else
	{
		renderBlock->programParams->setTextureParameter(s_handleOcean_WaveTexture0, m_evolvedSpectrumTextures[0]);
		renderBlock->programParams->setTextureParameter(s_handleOcean_WaveTexture1, m_evolvedSpectrumTextures[1]);
		renderBlock->programParams->setTextureParameter(s_handleOcean_WaveTexture2, m_evolvedSpectrumTextures[2]);
		renderBlock->programParams->setTextureParameter(s_handleOcean_WaveTexture3, m_evolvedSpectrumTextures[3]);
	}

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
		transform);

	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(sp.priority, renderBlock);
}

bool OceanComponent::createTileBuffers(uint32_t tileCount)
{
	safeDestroy(m_drawBuffer);
	safeDestroy(m_culledDrawBuffer);
	safeDestroy(m_dataBuffer);

	const uint32_t alignedTileCount = alignUp(tileCount, 16);

	m_drawBuffer = m_renderSystem->createBuffer(
		render::BuStructured | render::BuIndirect,
		alignedTileCount * sizeof(render::IndexedIndirectDraw),
		true,
		T_FILE_LINE_W);
	if (!m_drawBuffer)
		return false;

	m_culledDrawBuffer = m_renderSystem->createBuffer(
		render::BuStructured | render::BuIndirect,
		alignedTileCount * sizeof(render::IndexedIndirectDraw),
		false,
		T_FILE_LINE_W);
	if (!m_culledDrawBuffer)
		return false;

	m_dataBuffer = m_renderSystem->createBuffer(
		render::BuStructured,
		alignedTileCount * sizeof(DrawData),
		true,
		T_FILE_LINE_W);
	if (!m_dataBuffer)
		return false;

	m_tileCapacity = alignedTileCount;
	return true;
}

void OceanComponent::updateTiles(const world::WorldRenderView& worldRenderView)
{
	View& view = m_view[worldRenderView.getIndex()];
	view.tiles.resize(0);
	view.culled = false;

	if (!m_owner)
		return;

	const Vector4 eyePosition = worldRenderView.getEyePosition();
	const float farZ = worldRenderView.getViewFrustum().getFarZ();
	const float surfaceY = m_owner->getTransform().translation().y() + m_elevation;

	// Tiles are laid out in camera centered, concentric, levels of detail. Each
	// level covers a square around the eye which is aligned to the grid of the next
	// coarser level; thus tiles of a level are either completely inside or outside
	// the finer level and never overlap.
	float lastMinX = 0.0f, lastMinZ = 0.0f, lastMaxX = 0.0f, lastMaxZ = 0.0f;
	for (int32_t lod = 0; lod < m_lodCount; ++lod)
	{
		const bool lastLod = (lod == m_lodCount - 1);
		const float tileSize = m_tileSize * float(1 << lod);
		const float alignment = tileSize * 2.0f;

		// Half extent of this level; the coarsest level always reach the far plane.
		float extent = tileSize * 2.0f;
		if (lastLod)
			extent = max(extent, farZ + tileSize);

		const float minX = std::floor((eyePosition.x() - extent) / alignment) * alignment;
		const float minZ = std::floor((eyePosition.z() - extent) / alignment) * alignment;
		const float maxX = std::ceil((eyePosition.x() + extent) / alignment) * alignment;
		const float maxZ = std::ceil((eyePosition.z() + extent) / alignment) * alignment;

		for (float tz = minZ; tz < maxZ - tileSize * 0.5f; tz += tileSize)
		{
			for (float tx = minX; tx < maxX - tileSize * 0.5f; tx += tileSize)
			{
				// Skip tiles covered by finer level.
				if (lod > 0)
				{
					if (tx >= lastMinX && tx + tileSize <= lastMaxX && tz >= lastMinZ && tz + tileSize <= lastMaxZ)
						continue;
				}

				const Aabb3 aabb(
					Vector4(tx, surfaceY - c_maxWaveHeight, tz, 1.0f),
					Vector4(tx + tileSize, surfaceY + c_maxWaveHeight, tz + tileSize, 1.0f));

				float distance;
				if (!worldRenderView.isBoxVisible(aabb, Transform::identity(), distance))
					continue;

				// Edges facing a coarser level need to be stitched.
				uint32_t stitchMask = 0;
				if (!lastLod)
				{
					if (tx <= minX)
						stitchMask |= SeLeft;
					if (tx + tileSize >= maxX)
						stitchMask |= SeRight;
					if (tz <= minZ)
						stitchMask |= SeNear;
					if (tz + tileSize >= maxZ)
						stitchMask |= SeFar;
				}

				Tile& tile = view.tiles.push_back();
				tile.origin = Vector4(tx, 0.0f, tz, tileSize);
				tile.aabb = aabb;
				tile.stitchMask = stitchMask;
				tile.distance = distance;
			}
		}

		lastMinX = minX;
		lastMinZ = minZ;
		lastMaxX = maxX;
		lastMaxZ = maxZ;
	}

	// Sort tiles front to back.
	std::sort(view.tiles.begin(), view.tiles.end(), [](const Tile& lh, const Tile& rh) {
		return lh.distance < rh.distance;
	});

	// Ensure buffers are large enough.
	if ((uint32_t)view.tiles.size() > m_tileCapacity)
	{
		if (!createTileBuffers((uint32_t)view.tiles.size()))
		{
			view.tiles.resize(0);
			return;
		}
	}

	// Update indirect draw and per tile data buffers.
	auto draw = (render::IndexedIndirectDraw*)m_drawBuffer->lock();
	auto data = (DrawData*)m_dataBuffer->lock();
	if (!draw || !data)
	{
		if (draw)
			m_drawBuffer->unlock();
		if (data)
			m_dataBuffer->unlock();
		view.tiles.resize(0);
		return;
	}

	for (const auto& tile : view.tiles)
	{
		const auto& p = m_primitives[tile.stitchMask];

		draw->indexCount = p.getVertexCount();
		draw->instanceCount = 1;
		draw->firstIndex = p.offset;
		draw->vertexOffset = 0;
		draw->firstInstance = 0;
		draw++;

		tile.origin.storeUnaligned(data->patchOrigin);
		Vector4::zero().storeUnaligned(data->surfaceOffset);
		tile.aabb.mn.storeUnaligned(data->patchBoundingBoxMn);
		tile.aabb.mx.storeUnaligned(data->patchBoundingBoxMx);
		data++;
	}

	m_drawBuffer->unlock();
	m_dataBuffer->unlock();
}

}
