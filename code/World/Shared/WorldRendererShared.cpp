/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Timer/Profiler.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/ScreenRenderer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Render/Image2/ImageGraphData.h"
#include "Resource/IResourceManager.h"
#include "World/IEntityRenderer.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/Entity/LightComponent.h"
#include "World/Entity/ProbeComponent.h"
#include "World/Shared/WorldRendererShared.h"
#include "World/Shared/WorldRenderPassShared.h"
#include "World/Shared/Passes/AmbientOcclusionPass.h"
#include "World/Shared/Passes/GBufferPass.h"
#include "World/Shared/Passes/PostProcessPass.h"
#include "World/Shared/Passes/ReflectionsPass.h"
#include "World/Shared/Passes/VelocityPass.h"

namespace traktor::world
{
	namespace
	{

Ref< render::ITexture > create1x1Texture(render::IRenderSystem* renderSystem, uint32_t value)
{
	render::SimpleTextureCreateDesc stcd = {};
	stcd.width = 1;
	stcd.height = 1;
	stcd.mipCount = 1;
	stcd.format = render::TfR8G8B8A8;
	stcd.sRGB = false;
	stcd.immutable = true;
	stcd.initialData[0].data = &value;
	stcd.initialData[0].pitch = 4;
	return renderSystem->createSimpleTexture(stcd, T_FILE_LINE_W);
}

Ref< render::ITexture > createCubeTexture(render::IRenderSystem* renderSystem, uint32_t value)
{
	render::CubeTextureCreateDesc ctcd = {};
	ctcd.side = 1;
	ctcd.mipCount = 1;
	ctcd.format = render::TfR8G8B8A8;
	ctcd.sRGB = false;
	ctcd.immutable = true;
	ctcd.initialData[0].data = &value;
	ctcd.initialData[0].pitch = 4;
	ctcd.initialData[1].data = &value;
	ctcd.initialData[1].pitch = 4;
	ctcd.initialData[2].data = &value;
	ctcd.initialData[2].pitch = 4;
	ctcd.initialData[3].data = &value;
	ctcd.initialData[3].pitch = 4;
	ctcd.initialData[4].data = &value;
	ctcd.initialData[4].pitch = 4;
	ctcd.initialData[5].data = &value;
	ctcd.initialData[0].pitch = 4;
	return renderSystem->createCubeTexture(ctcd, T_FILE_LINE_W);
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRendererShared", WorldRendererShared, IWorldRenderer)

bool WorldRendererShared::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const WorldCreateDesc& desc
)
{
	m_entityRenderers = desc.entityRenderers;

	// Store settings.
	m_settings = *desc.worldRenderSettings;
	m_shadowsQuality = desc.quality.shadows;
	m_sharedDepthStencil = desc.sharedDepthStencil;

	m_imageGraphContext = new render::ImageGraphContext();
	
	// Create screen renderer.
	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	// Lights struct buffer.
	m_lightSBuffer = renderSystem->createBuffer(
		render::BuStructured,
		c_maxLightCount,
		sizeof(LightShaderData),
		true
	);
	if (!m_lightSBuffer)
		return false;

	// Tile light index array buffer.
	m_lightIndexSBuffer = renderSystem->createBuffer(
		render::BuStructured,
		ClusterDimXY * ClusterDimXY * ClusterDimZ * MaxLightsPerCluster,
		sizeof(LightIndexShaderData),
		true
	);
	if (!m_lightIndexSBuffer)
		return false;

	// Tile cluster buffer.
	m_tileSBuffer = renderSystem->createBuffer(
		render::BuStructured,
		ClusterDimXY * ClusterDimXY * ClusterDimZ,
		sizeof(TileShaderData),
		true
	);
	if (!m_tileSBuffer)
		return false;

	// Create default value textures.
	m_blackTexture = create1x1Texture(renderSystem, 0x00000000);
	m_whiteTexture = create1x1Texture(renderSystem, 0xffffffff);
	m_blackCubeTexture = createCubeTexture(renderSystem, 0x00000000);

	// Create shared passes.
	m_gbufferPass = new GBufferPass(m_settings, m_entityRenderers, m_sharedDepthStencil);

	m_velocityPass = new VelocityPass(m_settings, m_entityRenderers, m_sharedDepthStencil);
	if (!m_velocityPass->create(resourceManager, renderSystem, desc))
		return false;

	m_ambientOcclusionPass = new AmbientOcclusionPass(m_settings, m_entityRenderers, m_sharedDepthStencil);
	if (!m_ambientOcclusionPass->create(resourceManager, renderSystem, desc))
		return false;

	m_reflectionsPass = new ReflectionsPass(m_settings, m_entityRenderers, m_sharedDepthStencil);
	if (!m_reflectionsPass->create(resourceManager, renderSystem, desc))
		return false;

	m_postProcessPass = new PostProcessPass(m_settings, m_entityRenderers, m_sharedDepthStencil);
	if (!m_postProcessPass->create(resourceManager, renderSystem, desc))
		return false;

	return true;
}

void WorldRendererShared::destroy()
{
	safeDestroy(m_lightSBuffer);
	safeDestroy(m_lightIndexSBuffer);
	safeDestroy(m_tileSBuffer);
	safeDestroy(m_screenRenderer);
	safeDestroy(m_blackTexture);
	safeDestroy(m_whiteTexture);
	m_postProcessPass = nullptr;
	m_reflectionsPass = nullptr;
	m_ambientOcclusionPass = nullptr;
	m_velocityPass = nullptr;
	m_gbufferPass = nullptr;
}

render::ImageGraphContext* WorldRendererShared::getImageGraphContext() const
{
	return m_imageGraphContext;
}

void WorldRendererShared::setupTileDataPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId
)
{
	T_PROFILER_SCOPE(L"WorldRendererShared setupTileDataPass");
#if defined(T_WORLD_USE_TILE_JOB)
	// Enqueue light clustering as a job, is synchronized in before rendering.
	m_tileJob = JobManager::getInstance().add([=]() {
#endif
		const auto& viewFrustum = worldRenderView.getViewFrustum();

		TileShaderData* tileShaderData = (TileShaderData*)m_tileSBuffer->lock();
		std::memset(tileShaderData, 0, ClusterDimXY * ClusterDimXY * ClusterDimZ * sizeof(TileShaderData));

		LightIndexShaderData* lightIndexShaderData = (LightIndexShaderData*)m_lightIndexSBuffer->lock();
		std::memset(lightIndexShaderData, 0, ClusterDimXY * ClusterDimXY * ClusterDimZ * MaxLightsPerCluster * sizeof(LightIndexShaderData));

		StaticVector< Vector4, c_maxLightCount > lightPositions;
		StaticVector< int32_t, c_maxLightCount > sliceLights;

		// Calculate positions of lights in view space.
		for (const auto& light : m_gatheredView.lights)
		{
			if (light)
			{
				const Vector4 lightPosition = light->getTransform().translation().xyz1();
				lightPositions.push_back(worldRenderView.getView() * lightPosition);
			}
			else
				lightPositions.push_back(Vector4::zero());
		}

		// Update tile data.
		const Scalar dx(1.0f / ClusterDimXY);
		const Scalar dy(1.0f / ClusterDimXY);
		const Scalar dz(1.0f / ClusterDimZ);

		const Vector4& tl = viewFrustum.corners[0];
		const Vector4& tr = viewFrustum.corners[1];
		const Vector4& bl = viewFrustum.corners[3];

		const Vector4 vx = tr - tl;
		const Vector4 vy = bl - tl;

		const Scalar vnz = viewFrustum.getNearZ();
		const Scalar vfz = viewFrustum.getFarZ();

		// Calculate XY tile frustums.
		Frustum tileFrustums[ClusterDimXY * ClusterDimXY];
		for (int32_t y = 0; y < ClusterDimXY; ++y)
		{
			const Scalar fy = Scalar((float)y) * dy;
			for (int32_t x = 0; x < ClusterDimXY; ++x)
			{
				const Scalar fx = Scalar((float)x) * dx;
				
				const Vector4 a = tl + vx * fx + vy * fy;
				const Vector4 b = tl + vx * (fx + dx) + vy * fy;
				const Vector4 c = tl + vx * (fx + dx) + vy * (fy + dy);
				const Vector4 d = tl + vx * fx + vy * (fy + dy);
				
				auto& tileFrustum = tileFrustums[x + y * ClusterDimXY];
				tileFrustum.planes[Frustum::PsLeft] = Plane(Vector4::zero(), d, a);
				tileFrustum.planes[Frustum::PsRight] = Plane(Vector4::zero(), b, c);
				tileFrustum.planes[Frustum::PsBottom] = Plane(Vector4::zero(), c, d);
				tileFrustum.planes[Frustum::PsTop] = Plane(Vector4::zero(), a, b);
			}
		}

		// Group lights per cluster.
		uint32_t lightOffset = 0;
		for (int32_t z = 0; z < ClusterDimZ; ++z)
		{
			const float fz = (float)z;
			const Scalar snz = vnz * power(vfz / vnz, Scalar(fz) / Scalar(ClusterDimZ));
			const Scalar sfz = vnz * power(vfz / vnz, Scalar(fz + 1.0f) / Scalar(ClusterDimZ));

			// Gather all lights intersecting slice.
			sliceLights.clear();
			for (uint32_t i = 0; i < m_gatheredView.lights.size(); ++i)
			{
				const auto light = m_gatheredView.lights[i];
				if (light == nullptr)
					continue;

				if (light->getLightType() == LightType::Directional)
				{
					sliceLights.push_back(i);
				}
				else if (light->getLightType() == LightType::Point)
				{
					const Scalar lr = light->getRange();				
					const Scalar lz = lightPositions[i].z();
					if (lz + lr >= snz && lz - lr <= sfz)
						sliceLights.push_back(i);
				}
				else if (light->getLightType() == LightType::Spot)
				{
					const Scalar lr = light->getRange();				
					const Scalar lz = lightPositions[i].z();
					if (lz + lr >= snz && lz - lr <= sfz)
					{
						//Frustum spotFrustum;
						//spotFrustum.buildPerspective(light.radius, 1.0f, 0.0f, light.range);

						//Vector4 p[4];
						//p[0] = lightPositions[i] + worldRenderView.getView() * spotFrustum.corners[4].xyz0();
						//p[1] = lightPositions[i] + worldRenderView.getView() * spotFrustum.corners[5].xyz0();
						//p[2] = lightPositions[i] + worldRenderView.getView() * spotFrustum.corners[6].xyz0();
						//p[3] = lightPositions[i] + worldRenderView.getView() * spotFrustum.corners[7].xyz0();

						//Range< Scalar > bb;
						//bb.min = lz;
						//bb.max = lz;
						//for (int i = 0; i < 4; ++i)
						//{
						//	bb.min = min(bb.min, p[i].z());
						//	bb.max = max(bb.max, p[i].z());
						//}
						//if (Range< Scalar >::intersection(bb, Range< Scalar >(snz, sfz)).delta() > 0.0_simd)
							sliceLights.push_back(i);
					}
				}
			}

			if (sliceLights.empty())
				continue;

			for (int32_t y = 0; y < ClusterDimXY; ++y)
			{
				for (int32_t x = 0; x < ClusterDimXY; ++x)
				{
					auto& tileFrustum = tileFrustums[x + y * ClusterDimXY];
					tileFrustum.planes[Frustum::PsNear] = Plane(Vector4(0.0f, 0.0f, 1.0f), snz);
					tileFrustum.planes[Frustum::PsFar] = Plane(Vector4(0.0f, 0.0f, -1.0f), -sfz);

					int32_t count = 0;
					for (uint32_t i = 0; i < sliceLights.size(); ++i)
					{
						const int32_t lightIndex = sliceLights[i];
						const auto light = m_gatheredView.lights[lightIndex];
						if (light->getLightType() == LightType::Directional)
						{
							lightIndexShaderData[lightOffset + count].lightIndex[0] = lightIndex;
							++count;
						}
						else if (light->getLightType() == LightType::Point)
						{
							if (tileFrustum.inside(lightPositions[lightIndex], light->getRange()) != Frustum::IrOutside)
							{
								lightIndexShaderData[lightOffset + count].lightIndex[0] = lightIndex;
								++count;
							}
						}
						else if (light->getLightType() == LightType::Spot)
						{
							// \fixme Implement frustum to frustum culling.
							if (tileFrustum.inside(lightPositions[lightIndex], light->getRange()) != Frustum::IrOutside)
							{
								lightIndexShaderData[lightOffset + count].lightIndex[0] = lightIndex;
								++count;
							}
						}
						if (count >= MaxLightsPerCluster)
							break;
					}

					const uint32_t tileOffset = x + y * ClusterDimXY + z * ClusterDimXY * ClusterDimXY;
					tileShaderData[tileOffset].lightOffsetAndCount[0] = (int32_t)lightOffset;
					tileShaderData[tileOffset].lightOffsetAndCount[1] = count;

					lightOffset += count;
				}
			}
		}

		m_lightIndexSBuffer->unlock();
		m_tileSBuffer->unlock();
#if defined(T_WORLD_USE_TILE_JOB)
	});
#endif
}

}
