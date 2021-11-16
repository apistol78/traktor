#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
#include "Core/Math/Range.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Core/Timer/Profiler.h"
#include "Render/Buffer.h"
#include "Render/ICubeTexture.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/ScreenRenderer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Render/Image2/ImageGraphData.h"
#include "Resource/IResourceManager.h"
#include "World/Entity.h"
#include "World/IEntityRenderer.h"
#include "World/IrradianceGrid.h"
#include "World/Packer.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldGatherContext.h"
#include "World/WorldHandles.h"
#include "World/WorldSetupContext.h"
#include "World/Entity/LightComponent.h"
#include "World/Entity/ProbeComponent.h"
#include "World/Forward/WorldRendererForward.h"
#include "World/Forward/WorldRenderPassForward.h"
#include "World/SMProj/UniformShadowProjection.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

const int32_t c_maxLightCount = 16;

const render::Handle s_handleVisualTargetSet[] =
{
	render::Handle(L"World_VisualTargetSet_Even"),
	render::Handle(L"World_VisualTargetSet_Odd")
};

const resource::Id< render::ImageGraph > c_velocityPrime(L"{CB34E98B-55C9-E447-BD59-5A1D91DCA88E}");
const resource::Id< render::ImageGraph > c_ambientOcclusionLow(L"{416745F9-93C7-8D45-AE28-F2823DEE636A}");
const resource::Id< render::ImageGraph > c_ambientOcclusionMedium(L"{5A3B0260-32F9-B343-BBA4-88BD932F917A}");
const resource::Id< render::ImageGraph > c_ambientOcclusionHigh(L"{45F9CD9F-C700-9942-BB36-443629C88748}");
const resource::Id< render::ImageGraph > c_ambientOcclusionUltra(L"{302E57C8-711D-094F-A764-75F76553E81B}");
const resource::Id< render::ImageGraph > c_antiAliasLow(L"{71D385F1-8364-C849-927F-5F1249F5DF92}");
const resource::Id< render::ImageGraph > c_antiAliasMedium(L"{D03B9566-EFA3-7A43-B3AD-F59DB34DEE96}");
const resource::Id< render::ImageGraph > c_antiAliasHigh(L"{C0316981-FA73-A34E-8135-1F596425688F}");
const resource::Id< render::ImageGraph > c_antiAliasUltra(L"{88E329C8-A2F3-7443-B73E-4E85C6ECACBE}");
const resource::Id< render::ImageGraph > c_gammaCorrection(L"{B1E8367D-91DD-D648-A44F-B86492169771}");
const resource::Id< render::ImageGraph > c_toneMapFixed(L"{1F20DAB5-22EB-B84C-92B0-71E94C1CE261}");
const resource::Id< render::ImageGraph > c_toneMapAdaptive(L"{BE19DE90-E010-A74D-AA3B-87FAC2A56946}");
const resource::Id< render::ImageGraph > c_screenReflections(L"{2F8EC56A-FD46-DF42-94B5-9DD676B8DD8A}");

resource::Id< render::ImageGraph > getAmbientOcclusionId(Quality quality)
{
	switch (quality)
	{
	default:
	case Quality::Disabled:
		return resource::Id< render::ImageGraph >();
	case Quality::Low:
		return c_ambientOcclusionLow;
	case Quality::Medium:
		return c_ambientOcclusionMedium;
	case Quality::High:
		return c_ambientOcclusionHigh;
	case Quality::Ultra:
		return c_ambientOcclusionUltra;
	}
}

resource::Id< render::ImageGraph > getAntiAliasId(Quality quality)
{
	switch (quality)
	{
	default:
	case Quality::Disabled:
		return resource::Id< render::ImageGraph >();
	case Quality::Low:
		return c_antiAliasLow;
	case Quality::Medium:
		return c_antiAliasMedium;
	case Quality::High:
		return c_antiAliasHigh;
	case Quality::Ultra:
		return c_antiAliasUltra;
	}
}

resource::Id< render::ImageGraph > getToneMapId(WorldRenderSettings::ExposureMode exposureMode)
{
	switch (exposureMode)
	{
	default:
	case WorldRenderSettings::EmFixed:
		return c_toneMapFixed;
	case WorldRenderSettings::EmAdaptive:
		return c_toneMapAdaptive;
	}
}

Ref< render::ISimpleTexture > create1x1Texture(render::IRenderSystem* renderSystem, uint32_t value)
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

Vector2 jitter(int32_t count)
{
	const Vector2 kernelSize(0.5f, 0.5f);
	return Vector2(
		(float)((count / 2) & 1) * kernelSize.x - kernelSize.x / 2.0f,
		(float)(      count & 1) * kernelSize.y - kernelSize.y / 2.0f
	);
}

		}
		
#pragma pack(1)
struct LightShaderData
{
	float typeRangeRadius[4];
	float position[4];
	float direction[4];
	float color[4];
	float viewToLight0[4];
	float viewToLight1[4];
	float viewToLight2[4];
	float viewToLight3[4];
	float atlasTransform[4];
};
#pragma pack()

#pragma pack(1)
struct LightIndexShaderData
{
	int32_t lightIndex[4];
};
#pragma pack()

#pragma pack(1)
struct TileShaderData
{
	int32_t lightOffsetAndCount[4];
};
#pragma pack()

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRendererForward", 0, WorldRendererForward, IWorldRenderer)

bool WorldRendererForward::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const WorldCreateDesc& desc
)
{
	// Store settings.
	m_settings = *desc.worldRenderSettings;
	m_toneMapQuality = desc.quality.toneMap;
	m_motionBlurQuality = desc.quality.motionBlur;
	m_shadowsQuality = desc.quality.shadows;
	m_reflectionsQuality = desc.quality.reflections;
	m_ambientOcclusionQuality = desc.quality.ambientOcclusion;
	m_antiAliasQuality = desc.quality.antiAlias;
	m_gamma = desc.gamma;
	m_sharedDepthStencil = desc.sharedDepthStencil;

	// Create velocity prime processing; priming is also used by TAA.
	if (
		m_motionBlurQuality > Quality::Disabled ||
		m_antiAliasQuality >= Quality::Ultra
	)
	{
		if (!resourceManager->bind(c_velocityPrime, m_velocityPrime))
		{
			log::warning << L"Unable to create velocity prime process; disabled." << Endl;
			m_motionBlurQuality = Quality::Disabled;
		}
	}

	// Create ambient occlusion processing.
	if (m_ambientOcclusionQuality > Quality::Disabled)
	{
		resource::Id< render::ImageGraph > ambientOcclusion = getAmbientOcclusionId(m_ambientOcclusionQuality);
		if (!resourceManager->bind(ambientOcclusion, m_ambientOcclusion))
			log::warning << L"Unable to create ambient occlusion process; AO disabled." << Endl;
	}

	// Create antialias processing.
	if (m_antiAliasQuality > Quality::Disabled)
	{
		resource::Id< render::ImageGraph > antiAlias = getAntiAliasId(m_antiAliasQuality);
		if (!resourceManager->bind(antiAlias, m_antiAlias))
			log::warning << L"Unable to create antialias process; AA disabled." << Endl;
	}

	// Create "visual" post processing filter.
	if (desc.quality.imageProcess > Quality::Disabled)
	{
		const auto& visualImageGraph = desc.worldRenderSettings->imageProcess[(int32_t)desc.quality.imageProcess];
		if (!visualImageGraph.isNull())
		{
			if (!resourceManager->bind(visualImageGraph, m_visual))
				log::warning << L"Unable to create visual post processing; post processing disabled." << Endl;
		}
	}

	// Create gamma correction processing.
	if (std::abs(m_gamma - 1.0f) > FUZZY_EPSILON)
	{
		if (!resourceManager->bind(c_gammaCorrection, m_gammaCorrection))
			log::warning << L"Unable to create gamma correction process; gamma correction disabled." << Endl;
	}

	// Create tone map processing.
	if (m_toneMapQuality > Quality::Disabled)
	{
		resource::Id< render::ImageGraph > toneMap = getToneMapId(m_settings.exposureMode);
		if (!resourceManager->bind(toneMap, m_toneMap))
		{
			log::warning << L"Unable to create tone map process." << Endl;
			m_toneMapQuality = Quality::Disabled;
		}		
	}

	// Create screen reflections processing.
	if (m_reflectionsQuality >= Quality::Disabled)
	{
		if (!resourceManager->bind(c_screenReflections, m_screenReflections))
		{
			log::warning << L"Unable to create screen space reflections process." << Endl;
			m_reflectionsQuality = Quality::Disabled;
		}
	}

	m_imageGraphContext = new render::ImageGraphContext();

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

	const auto& shadowSettings = m_settings.shadowSettings[(int32_t)m_shadowsQuality];
	m_shadowAtlasPacker = new Packer(
		shadowSettings.resolution,
		shadowSettings.resolution
	);

	// Create irradiance grid.
	if (!m_settings.irradianceGrid.isNull())
	{
		if (!resourceManager->bind(m_settings.irradianceGrid, m_irradianceGrid))
			return false;
	}

	// Determine slice distances.
	for (int32_t i = 0; i < shadowSettings.cascadingSlices; ++i)
	{
		float ii = float(i) / shadowSettings.cascadingSlices;
		float log = powf(ii, shadowSettings.cascadingLambda);
		m_slicePositions[i] = lerp(m_settings.viewNearZ, shadowSettings.farZ, log);
	}
	m_slicePositions[shadowSettings.cascadingSlices] = shadowSettings.farZ;

	// Keep entity renderers.
	m_entityRenderers = desc.entityRenderers;

	// Create screen renderer.
	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	// Create default value textures.
	m_blackTexture = create1x1Texture(renderSystem, 0x00000000);
	m_whiteTexture = create1x1Texture(renderSystem, 0xffffffff);
	return true;
}

void WorldRendererForward::destroy()
{
	safeDestroy(m_lightSBuffer);
	safeDestroy(m_lightIndexSBuffer);
	safeDestroy(m_tileSBuffer);

	safeDestroy(m_screenRenderer);
	safeDestroy(m_blackTexture);
	safeDestroy(m_whiteTexture);

	m_irradianceGrid.clear();
}

void WorldRendererForward::setup(
	const WorldRenderView& immutableWorldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId
)
{
	//int32_t frame = m_count % (int32_t)m_frames.size();
	WorldRenderView worldRenderView = immutableWorldRenderView;

#if defined(T_WORLD_FORWARD_USE_TILE_JOB)
	// Ensure tile job is finished, this should never happen since it will indicate
	// previous frame hasn't been rendered.
	if (m_tileJob != nullptr)
	{
		m_tileJob->wait();
		m_tileJob = nullptr;
	}
#endif

	// Jitter projection for TAA, calculate jitter in clip space.
	if (m_antiAliasQuality >= Quality::Ultra)
	{
		Vector2 r = (jitter(m_count) * 2.0f) / worldRenderView.getViewSize();
		Matrix44 proj = immutableWorldRenderView.getProjection();
		proj = translate(r.x, r.y, 0.0f) * proj;
		worldRenderView.setProjection(proj);
	}

	// Gather active renderables for this frame.
	{
		T_PROFILER_SCOPE(L"WorldRendererForward gather");

		m_lights.resize(0);
		m_probes.resize(0);
		m_gathered.resize(0);

		WorldGatherContext(m_entityRenderers, rootEntity, [&](IEntityRenderer* entityRenderer, Object* renderable) {

			// Gather lights and probes separately as we need them for shadows and lighting.
			if (auto lightComponent = dynamic_type_cast< const LightComponent* >(renderable))
				m_lights.push_back(lightComponent);
			else if (auto probeComponent = dynamic_type_cast< const ProbeComponent* >(renderable))
				m_probes.push_back(probeComponent);

			m_gathered.push_back({ entityRenderer, renderable });

		}).gather(const_cast< Entity* >(rootEntity));

		if (m_lights.size() > c_maxLightCount)
			m_lights.resize(c_maxLightCount);
	}

	// Add additional passes by entity renderers.
	{
		T_PROFILER_SCOPE(L"WorldRendererForward setup extra passes");
		WorldSetupContext context(m_entityRenderers, rootEntity, renderGraph);

		for (auto gathered : m_gathered)
			gathered.entityRenderer->setup(context, worldRenderView, gathered.renderable);
	
		for (auto entityRenderer : m_entityRenderers->get())
			entityRenderer->setup(context);
	}

	// Add visual target sets.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = (m_sharedDepthStencil == nullptr) ? true : false;
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	rgtd.targets[0].colorFormat = render::TfR11G11B10F;
	auto visualReadTargetSetId = renderGraph.addPersistentTargetSet(L"History", s_handleVisualTargetSet[m_count % 2], rgtd, m_sharedDepthStencil, outputTargetSetId);
	auto visualWriteTargetSetId = renderGraph.addPersistentTargetSet(L"Visual", s_handleVisualTargetSet[(m_count + 1) % 2], rgtd, m_sharedDepthStencil, outputTargetSetId);

	// Add passes to render graph.
	setupTileDataPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		outputTargetSetId
	);

	auto gbufferTargetSetId = setupGBufferPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		outputTargetSetId
	);

	auto velocityTargetSetId = setupVelocityPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		outputTargetSetId,
		gbufferTargetSetId
	);

	auto ambientOcclusionTargetSetId = setupAmbientOcclusionPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		outputTargetSetId,
		gbufferTargetSetId
	);

	auto reflectionsTargetSetId = setupReflectionsPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		outputTargetSetId,
		gbufferTargetSetId,
		visualReadTargetSetId
	);

	render::handle_t shadowMapAtlasTargetSetId = 0;
	setupLightPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		outputTargetSetId,
		shadowMapAtlasTargetSetId
	);

	setupVisualPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		visualWriteTargetSetId,
		gbufferTargetSetId,
		ambientOcclusionTargetSetId,
		reflectionsTargetSetId,
		shadowMapAtlasTargetSetId
	);

	setupProcessPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		outputTargetSetId,
		gbufferTargetSetId,
		velocityTargetSetId,
		visualWriteTargetSetId,
		visualReadTargetSetId
	);

	m_count++;
}

render::ImageGraphContext* WorldRendererForward::getImageGraphContext() const
{
	return m_imageGraphContext;
}

void WorldRendererForward::setupTileDataPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId
)
{
	T_PROFILER_SCOPE(L"WorldRendererForward setupTileDataPass");
#if defined(T_WORLD_FORWARD_USE_TILE_JOB)
	// Enqueue light clustering as a job, is synchronized in before rendering.
	m_tileJob = JobManager::getInstance().add([=]() {
#endif
		const auto& viewFrustum = worldRenderView.getViewFrustum();
		const auto& lights = m_lights;

		TileShaderData* tileShaderData = (TileShaderData*)m_tileSBuffer->lock();
		std::memset(tileShaderData, 0, ClusterDimXY * ClusterDimXY * ClusterDimZ * sizeof(TileShaderData));

		LightIndexShaderData* lightIndexShaderData = (LightIndexShaderData*)m_lightIndexSBuffer->lock();
		std::memset(lightIndexShaderData, 0, ClusterDimXY * ClusterDimXY * ClusterDimZ * MaxLightsPerCluster * sizeof(LightIndexShaderData));

		uint32_t lightOffset = 0;

		StaticVector< Vector4, c_maxLightCount > lightPositions;
		StaticVector< int32_t, c_maxLightCount > sliceLights;

		// Calculate positions of lights in view space.
		for (const auto& light : lights)
		{
			Vector4 lightPosition = light->getTransform().translation().xyz1();
			lightPositions.push_back(worldRenderView.getView() * lightPosition);
		}

		// Update tile data.
		const Scalar dx(1.0f / ClusterDimXY);
		const Scalar dy(1.0f / ClusterDimXY);
		const Scalar dz(1.0f / ClusterDimZ);

		const Vector4& tl = viewFrustum.corners[0];
		const Vector4& tr = viewFrustum.corners[1];
		const Vector4& bl = viewFrustum.corners[3];

		Vector4 vx = tr - tl;
		Vector4 vy = bl - tl;

		Scalar vnz = viewFrustum.getNearZ();
		Scalar vfz = viewFrustum.getFarZ();

		// Calculate XY tile frustums.
		Frustum tileFrustums[ClusterDimXY * ClusterDimXY];
		for (int32_t y = 0; y < ClusterDimXY; ++y)
		{
			Scalar fy = Scalar((float)y) * dy;
			for (int32_t x = 0; x < ClusterDimXY; ++x)
			{
				Scalar fx = Scalar((float)x) * dx;
				
				Vector4 a = tl + vx * fx + vy * fy;
				Vector4 b = tl + vx * (fx + dx) + vy * fy;
				Vector4 c = tl + vx * (fx + dx) + vy * (fy + dy);
				Vector4 d = tl + vx * fx + vy * (fy + dy);
				
				auto& tileFrustum = tileFrustums[x + y * ClusterDimXY];
				tileFrustum.planes[Frustum::PsLeft] = Plane(Vector4::zero(), d, a);
				tileFrustum.planes[Frustum::PsRight] = Plane(Vector4::zero(), b, c);
				tileFrustum.planes[Frustum::PsBottom] = Plane(Vector4::zero(), c, d);
				tileFrustum.planes[Frustum::PsTop] = Plane(Vector4::zero(), a, b);
			}
		}

		// Group lights per cluster.
		for (int32_t z = 0; z < ClusterDimZ; ++z)
		{
			Scalar snz = vnz * power(vfz / vnz, Scalar(z) / Scalar(ClusterDimZ));
			Scalar sfz = vnz * power(vfz / vnz, Scalar(z + 1) / Scalar(ClusterDimZ));

			// Gather all lights intersecting slice.
			sliceLights.clear();
			for (uint32_t i = 0; i < lights.size(); ++i)
			{
				const auto light = lights[i];
				if (light->getLightType() == LightType::LtDirectional)
				{
					sliceLights.push_back(i);
				}
				else if (light->getLightType() == LightType::LtPoint)
				{
					Scalar lr = light->getRange();				
					Scalar lz = lightPositions[i].z();
					if (lz + lr >= snz && lz - lr <= sfz)
						sliceLights.push_back(i);
				}
				else if (light->getLightType() == LightType::LtSpot)
				{
					Scalar lr = light->getRange();				
					Scalar lz = lightPositions[i].z();
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
						int32_t lightIndex = sliceLights[i];
						const auto light = lights[lightIndex];
						if (light->getLightType() == LightType::LtDirectional)
						{
							lightIndexShaderData[lightOffset + count].lightIndex[0] = lightIndex;
							++count;
						}
						else if (light->getLightType() == LightType::LtPoint)
						{
							if (tileFrustum.inside(lightPositions[lightIndex], light->getRange()) != Frustum::IrOutside)
							{
								lightIndexShaderData[lightOffset + count].lightIndex[0] = lightIndex;
								++count;
							}
						}
						else if (light->getLightType() == LightType::LtSpot)
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
#if defined(T_WORLD_FORWARD_USE_TILE_JOB)
	});
#endif
}

render::handle_t WorldRendererForward::setupGBufferPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId
) const
{
	T_PROFILER_SCOPE(L"WorldRendererForward setupGBufferPass");
	const float clearZ = m_settings.viewFarZ;

	// Add GBuffer target set.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 2;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = (m_sharedDepthStencil == nullptr) ? true : false;
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	rgtd.targets[0].colorFormat = render::TfR16G16F;	// Depth (R), Roughness (G)
	rgtd.targets[1].colorFormat = render::TfR11G11B10F;	// Normals (RGB)
	auto gbufferTargetSetId = renderGraph.addTransientTargetSet(L"GBuffer", rgtd, m_sharedDepthStencil, outputTargetSetId);

	// Add GBuffer render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"GBuffer");
	
	render::Clear clear;
	clear.mask = render::CfColor | render::CfDepth | render::CfStencil;
	clear.colors[0] = Color4f(clearZ, 1.0f, 0.0f, 0.0f);
	clear.colors[1] = Color4f(0.5f, 0.5f, 0.0f, 0.0f);
	clear.depth = 1.0f;
	clear.stencil = 0;
	rp->setOutput(gbufferTargetSetId, clear, render::TfNone, render::TfAll);

	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			WorldBuildContext wc(
				m_entityRenderers,
				rootEntity,
				renderContext
			);

			auto sharedParams = wc.getRenderContext()->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(renderContext);
			sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
			sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
			sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
			sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
			sharedParams->endParameters(renderContext);

			WorldRenderPassForward gbufferPass(
				s_techniqueForwardGBufferWrite,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::PfFirst
			);

			T_ASSERT(!renderContext->havePendingDraws());

			for (auto gathered : m_gathered)
				gathered.entityRenderer->build(wc, worldRenderView, gbufferPass, gathered.renderable);
	
			for (auto entityRenderer : m_entityRenderers->get())
				entityRenderer->build(wc, worldRenderView, gbufferPass);

			renderContext->merge(render::RpAll);		
		}
	);

	renderGraph.addPass(rp);
	return gbufferTargetSetId;
}

render::handle_t WorldRendererForward::setupVelocityPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId,
	render::handle_t gbufferTargetSetId
) const
{
	T_PROFILER_SCOPE(L"WorldRendererForward setupVelocityPass");

	// Add Velocity target set.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = (m_sharedDepthStencil == nullptr) ? true : false;
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	rgtd.targets[0].colorFormat = render::TfR32G32F;
	auto velocityTargetSetId = renderGraph.addTransientTargetSet(L"Velocity", rgtd, m_sharedDepthStencil, outputTargetSetId);

	// Add Velocity render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Velocity");
	
	if (m_velocityPrime)
	{
		render::ImageGraphView view;

		view.viewFrustum = worldRenderView.getViewFrustum();
		view.view = worldRenderView.getLastView() * worldRenderView.getView().inverse();
		view.projection = worldRenderView.getProjection();
		view.deltaTime = worldRenderView.getDeltaTime();

		m_imageGraphContext->associateTextureTargetSet(s_handleInputDepth, gbufferTargetSetId, 0);

		m_velocityPrime->addPasses(
			m_screenRenderer,
			renderGraph,
			rp,
			*m_imageGraphContext,
			view
		);
	}

	rp->setOutput(velocityTargetSetId, render::TfDepth, render::TfColor | render::TfDepth);

	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			WorldBuildContext wc(
				m_entityRenderers,
				rootEntity,
				renderContext
			);

			auto sharedParams = renderContext->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(renderContext);
			sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
			sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
			sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
			sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
			sharedParams->endParameters(renderContext);

			WorldRenderPassForward velocityPass(
				s_techniqueVelocityWrite,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::PfNone
			);

			for (auto gathered : m_gathered)
				gathered.entityRenderer->build(wc, worldRenderView, velocityPass, gathered.renderable);
	
			for (auto entityRenderer : m_entityRenderers->get())
				entityRenderer->build(wc, worldRenderView, velocityPass);

			renderContext->merge(render::RpAll);
		}
	);

	renderGraph.addPass(rp);
	return velocityTargetSetId;
}

render::handle_t WorldRendererForward::setupAmbientOcclusionPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId,
	render::handle_t gbufferTargetSetId
) const
{
	render::ImageGraphView view;

	if (m_ambientOcclusion == nullptr)
		return 0;

	T_PROFILER_SCOPE(L"WorldRendererForward setupAmbientOcclusionPass");

	// Add ambient occlusion target set.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = false;
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	rgtd.targets[0].colorFormat = render::TfR8;			// Ambient occlusion (R)
	auto ambientOcclusionTargetSetId = renderGraph.addTransientTargetSet(L"Ambient occlusion", rgtd, m_sharedDepthStencil, outputTargetSetId);

	// Add ambient occlusion render pass.
	view.viewFrustum = worldRenderView.getViewFrustum();
	view.view = worldRenderView.getView();
	view.projection = worldRenderView.getProjection();

	m_imageGraphContext->associateTextureTargetSet(s_handleInputDepth, gbufferTargetSetId, 0);
	m_imageGraphContext->associateTextureTargetSet(s_handleInputNormal, gbufferTargetSetId, 1);

	Ref< render::RenderPass > rp = new render::RenderPass(L"Ambient occlusion");
	m_ambientOcclusion->addPasses(
		m_screenRenderer,
		renderGraph,
		rp,
		*m_imageGraphContext,
		view
	);

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	rp->setOutput(ambientOcclusionTargetSetId, clear, render::TfNone, render::TfColor);

	renderGraph.addPass(rp);
	return ambientOcclusionTargetSetId;
}

render::handle_t WorldRendererForward::setupReflectionsPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId,
	render::handle_t gbufferTargetSetId,
	render::handle_t visualReadTargetSetId
) const
{
	if (m_reflectionsQuality == Quality::Disabled)
		return 0;

	T_PROFILER_SCOPE(L"WorldRendererForward setupReflectionsPass");

	// Add reflections target.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = false;
	rgtd.ignoreStencil = true;
	rgtd.targets[0].colorFormat = render::TfR11G11B10F;

	switch (m_reflectionsQuality)
	{
	default:
	case Quality::Low:
		rgtd.referenceWidthDenom = 2;	// 50%
		rgtd.referenceHeightDenom = 2;
		break;

	case Quality::Medium:
		rgtd.referenceWidthMul = 2;		// 67%
		rgtd.referenceWidthDenom = 3;
		rgtd.referenceHeightMul = 2;
		rgtd.referenceHeightDenom = 3;
		break;

	case Quality::High:
		rgtd.referenceWidthMul = 4;		// 80%
		rgtd.referenceWidthDenom = 5;
		rgtd.referenceHeightMul = 4;
		rgtd.referenceHeightDenom = 5;
		break;

	case Quality::Ultra:
		rgtd.referenceWidthDenom = 1;	// 100%
		rgtd.referenceHeightDenom = 1;
		break;
	}

	auto reflectionsTargetSetId = renderGraph.addTransientTargetSet(L"Reflections", rgtd, m_sharedDepthStencil, outputTargetSetId);

	// Add reflections render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Reflections");

	rp->addInput(gbufferTargetSetId);
	rp->addInput(visualReadTargetSetId);

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	rp->setOutput(reflectionsTargetSetId, clear, render::TfNone, render::TfColor);
	
	 // Render screenspace reflections.
	render::ImageGraphView view;
	view.viewFrustum = worldRenderView.getViewFrustum();
	view.view = worldRenderView.getView();
	view.projection = worldRenderView.getProjection();
	view.deltaTime = worldRenderView.getDeltaTime();

	m_imageGraphContext->associateTextureTargetSet(s_handleInputColorLast, visualReadTargetSetId, 0);
	m_imageGraphContext->associateTextureTargetSet(s_handleInputDepth, gbufferTargetSetId, 0);
	m_imageGraphContext->associateTextureTargetSet(s_handleInputNormal, gbufferTargetSetId, 1);
	m_imageGraphContext->associateTextureTargetSet(s_handleInputRoughness, gbufferTargetSetId, 1);

	m_screenReflections->addPasses(
		m_screenRenderer,
		renderGraph,
		rp,
		*m_imageGraphContext,
		view
	);

	renderGraph.addPass(rp);
	return reflectionsTargetSetId;
}

void WorldRendererForward::setupLightPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId,
	render::handle_t& outShadowMapAtlasTargetSetId
)
{
	T_PROFILER_SCOPE(L"WorldRendererForward setupLightPass");

	const UniformShadowProjection shadowProjection(1024);
	const auto& shadowSettings = m_settings.shadowSettings[(int32_t)m_shadowsQuality];
	const bool shadowsEnable = (bool)(m_shadowsQuality != Quality::Disabled);
	const auto& lights = m_lights;

	LightShaderData* lightShaderData = (LightShaderData*)m_lightSBuffer->lock();
	if (!lightShaderData)
		return;

	// Reset this frame's atlas packer.
	auto shadowAtlasPacker = m_shadowAtlasPacker;
	shadowAtlasPacker->reset();

	Matrix44 view = worldRenderView.getView();
	Matrix44 viewInverse = worldRenderView.getView().inverse();
	Frustum viewFrustum = worldRenderView.getViewFrustum();
	
	// Find cascade shadow light.
	int32_t lightCascadeIndex = -1;
	if (shadowsEnable && shadowSettings.cascadingSlices > 0)
	{
		for (int32_t i = 0; i < (int32_t)lights.size(); ++i)
		{
			const auto light = lights[i];
			if (light->getCastShadow() && light->getLightType() == LightType::LtDirectional)
			{
				lightCascadeIndex = i;
				break;
			}
		}
	}

	// Find atlas shadow lights.
	StaticVector< int32_t, 16 > lightAtlasIndices;
	if (shadowsEnable)
	{
		for (int32_t i = 0; i < (int32_t)lights.size(); ++i)
		{
			const auto light = lights[i];
			if (light->getCastShadow() && light->getLightType() == LightType::LtSpot)
				lightAtlasIndices.push_back(i);
		}
	}

	// Write all lights to sbuffer; without shadow map information.
	for (int32_t i = 0; i < (int32_t)lights.size(); ++i)
	{
		const auto light = lights[i];
		auto* lsd = &lightShaderData[i];

		lsd->typeRangeRadius[0] = (float)light->getLightType();
		lsd->typeRangeRadius[1] = light->getRange();
		lsd->typeRangeRadius[2] = std::cos((light->getRadius() - deg2rad(8.0f)) / 2.0f);
		lsd->typeRangeRadius[3] = std::cos(light->getRadius() / 2.0f);

		Matrix44 lightTransform = view * light->getTransform().toMatrix44();
		lightTransform.translation().xyz1().storeUnaligned(lsd->position);
		lightTransform.axisY().xyz0().storeUnaligned(lsd->direction);
		light->getColor().storeUnaligned(lsd->color);

		Vector4::zero().storeUnaligned(lsd->viewToLight0);
		Vector4::zero().storeUnaligned(lsd->viewToLight1);
		Vector4::zero().storeUnaligned(lsd->viewToLight2);
		Vector4::zero().storeUnaligned(lsd->viewToLight3);
	}

	// If shadow casting directional light found add cascade shadow map pass
	// and update light sbuffer.
	if (shadowsEnable)
	{
		const int32_t cascadingSlices = lightCascadeIndex >= 0 ? shadowSettings.cascadingSlices : 0;
		const int32_t shmw = shadowSettings.resolution * (cascadingSlices + 1);
		const int32_t shmh = shadowSettings.resolution;
		const int32_t sliceDim = shadowSettings.resolution;
		const int32_t atlasOffset = shadowSettings.resolution * cascadingSlices;

		// Add shadow map target.
		render::RenderGraphTargetSetDesc rgtd;
		rgtd.count = 0;
		rgtd.width = shmw;
		rgtd.height = shmh;
		rgtd.createDepthStencil = true;
		rgtd.usingPrimaryDepthStencil = false;
		rgtd.usingDepthStencilAsTexture = true;
		rgtd.ignoreStencil = true;
		outShadowMapAtlasTargetSetId = renderGraph.addTransientTargetSet(L"Shadow map atlas", rgtd);

		// Add shadow map render passes.
		Ref< render::RenderPass > rp = new render::RenderPass(L"Shadow map");

		render::Clear clear;
		clear.mask = render::CfDepth;
		clear.depth = 1.0f;
		rp->setOutput(outShadowMapAtlasTargetSetId, clear, render::TfNone, render::TfDepth);

		if (lightCascadeIndex >= 0)
		{
			const auto light = lights[lightCascadeIndex];
			Transform lightTransform = light->getTransform();
			Vector4 lightPosition = lightTransform.translation().xyz1();
			Vector4 lightDirection = lightTransform.axisY().xyz0();

			auto* lsd = &lightShaderData[lightCascadeIndex];

			for (int32_t slice = 0; slice < shadowSettings.cascadingSlices; ++slice)
			{
				Scalar zn(max(m_slicePositions[slice], m_settings.viewNearZ));
				Scalar zf(min(m_slicePositions[slice + 1], shadowSettings.farZ));

				// Create sliced view frustum.
				Frustum sliceViewFrustum = viewFrustum;
				sliceViewFrustum.setNearZ(zn);
				sliceViewFrustum.setFarZ(zf);

				// Calculate shadow map projection.
				Matrix44 shadowLightView;
				Matrix44 shadowLightProjection;
				Frustum shadowFrustum;

				shadowProjection.calculate(
					viewInverse,
					lightPosition,
					lightDirection,
					sliceViewFrustum,
					shadowSettings.farZ,
					shadowSettings.quantizeProjection,
					shadowLightView,
					shadowLightProjection,
					shadowFrustum
				);

				Matrix44 viewToLightSpace = shadowLightProjection * shadowLightView * viewInverse;
				viewToLightSpace.axisX().storeUnaligned(lsd->viewToLight0);
				viewToLightSpace.axisY().storeUnaligned(lsd->viewToLight1);
				viewToLightSpace.axisZ().storeUnaligned(lsd->viewToLight2);
				viewToLightSpace.translation().storeUnaligned(lsd->viewToLight3);

				// Write slice coordinates to shaders.
				Vector4(
					(float)(slice * sliceDim) / shmw,
					0.0f,
					(float)sliceDim / shmw,
					1.0f
				).storeUnaligned(lsd->atlasTransform);

				rp->addBuild(
					[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
					{
						WorldBuildContext wc(
							m_entityRenderers,
							rootEntity,
							renderContext
						);

						// Render shadow map.
						WorldRenderView shadowRenderView;
						shadowRenderView.setProjection(shadowLightProjection);
						shadowRenderView.setView(shadowLightView, shadowLightView);
						shadowRenderView.setViewFrustum(shadowFrustum);
						shadowRenderView.setCullFrustum(shadowFrustum);
						shadowRenderView.setTimes(
							worldRenderView.getTime(),
							worldRenderView.getDeltaTime(),
							worldRenderView.getInterval()
						);

						// Set viewport to current cascade.
						auto svrb = renderContext->alloc< render::SetViewportRenderBlock >();
						svrb->viewport = render::Viewport(
							slice * sliceDim,
							0,
							sliceDim,
							sliceDim,
							0.0f,
							1.0f
						);
						renderContext->enqueue(svrb);	

						// Render entities into shadow map.
						auto sharedParams = renderContext->alloc< render::ProgramParameters >();
						sharedParams->beginParameters(renderContext);
						sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
						sharedParams->setMatrixParameter(s_handleProjection, shadowLightProjection);
						sharedParams->setMatrixParameter(s_handleView, shadowLightView);
						sharedParams->setMatrixParameter(s_handleViewInverse, shadowLightView.inverse());
						sharedParams->endParameters(renderContext);

						WorldRenderPassForward shadowPass(
							s_techniqueShadow,
							sharedParams,
							shadowRenderView,
							IWorldRenderPass::PfNone
						);

						T_ASSERT(!renderContext->havePendingDraws());

						for (auto gathered : m_gathered)
							gathered.entityRenderer->build(wc, shadowRenderView, shadowPass, gathered.renderable);
	
						for (auto entityRenderer : m_entityRenderers->get())
							entityRenderer->build(wc, shadowRenderView, shadowPass);

						renderContext->merge(render::RpAll);
					}
				);
			}
		}

		for (int32_t lightAtlasIndex : lightAtlasIndices)
		{
			const auto light = lights[lightAtlasIndex];
			Transform lightTransform = light->getTransform();
			Vector4 lightPosition = lightTransform.translation().xyz1();
			Vector4 lightDirection = lightTransform.axisY().xyz0();

			auto* lsd = &lightShaderData[lightAtlasIndex];

			// Calculate shadow map projection.
			Matrix44 shadowLightView;
			Matrix44 shadowLightProjection;
			Frustum shadowFrustum;

			shadowFrustum.buildPerspective(light->getRadius(), 1.0f, 0.1f, light->getRange());
			shadowLightProjection = perspectiveLh(light->getRadius(), 1.0f, 0.1f, light->getRange());

			Vector4 lightAxisX, lightAxisY, lightAxisZ;
			lightAxisZ = -lightDirection;
			lightAxisX = cross(viewInverse.axisZ(), lightAxisZ).normalized();
			lightAxisY = cross(lightAxisX, lightAxisZ).normalized();

			shadowLightView = Matrix44(
				lightAxisX,
				lightAxisY,
				lightAxisZ,
				lightPosition
			);
			shadowLightView = shadowLightView.inverse();

			Matrix44 viewToLightSpace = shadowLightProjection * shadowLightView * viewInverse;
			viewToLightSpace.axisX().storeUnaligned(lsd->viewToLight0);
			viewToLightSpace.axisY().storeUnaligned(lsd->viewToLight1);
			viewToLightSpace.axisZ().storeUnaligned(lsd->viewToLight2);
			viewToLightSpace.translation().storeUnaligned(lsd->viewToLight3);

			// Calculate size of shadow region based on distance from eye.
			float distance = (worldRenderView.getEyePosition() - lightPosition).xyz0().length();
			int32_t denom = (int32_t)std::floor(distance / 4.0f);
			int32_t atlasSize = 128 >> std::min(denom, 4);
					
			Packer::Rectangle atlasRect;
			if (!shadowAtlasPacker->insert(atlasSize, atlasSize, atlasRect))
				return;

			// Write atlas coordinates to shaders.
			Vector4(
				(float)(atlasOffset + atlasRect.x) / shmw,
				(float)atlasRect.y / shmh,
				(float)atlasRect.width / shmw,
				(float)atlasRect.height / shmh
			).storeUnaligned(lsd->atlasTransform);	

			rp->addBuild(
				[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
				{
					WorldBuildContext wc(
						m_entityRenderers,
						rootEntity,
						renderContext
					);

					// Render shadow map.
					WorldRenderView shadowRenderView;
					shadowRenderView.setProjection(shadowLightProjection);
					shadowRenderView.setView(shadowLightView, shadowLightView);
					shadowRenderView.setViewFrustum(shadowFrustum);
					shadowRenderView.setCullFrustum(shadowFrustum);
					shadowRenderView.setTimes(
						worldRenderView.getTime(),
						worldRenderView.getDeltaTime(),
						worldRenderView.getInterval()
					);

					// Set viewport to light atlas slot.
					auto svrb = renderContext->alloc< render::SetViewportRenderBlock >();
					svrb->viewport = render::Viewport(
						atlasOffset + atlasRect.x,
						atlasRect.y,
						atlasRect.width,
						atlasRect.height,
						0.0f,
						1.0f
					);
					renderContext->enqueue(svrb);	

					// Render entities into shadow map.
					auto sharedParams = renderContext->alloc< render::ProgramParameters >();
					sharedParams->beginParameters(renderContext);
					sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
					sharedParams->setMatrixParameter(s_handleProjection, shadowLightProjection);
					sharedParams->setMatrixParameter(s_handleView, shadowLightView);
					sharedParams->setMatrixParameter(s_handleViewInverse, shadowLightView.inverse());
					sharedParams->endParameters(renderContext);

					WorldRenderPassForward shadowPass(
						s_techniqueShadow,
						sharedParams,
						shadowRenderView,
						IWorldRenderPass::PfNone
					);

					T_ASSERT(!renderContext->havePendingDraws());

					for (auto gathered : m_gathered)
						gathered.entityRenderer->build(wc, shadowRenderView, shadowPass, gathered.renderable);
	
					for (auto entityRenderer : m_entityRenderers->get())
						entityRenderer->build(wc, shadowRenderView, shadowPass);

					renderContext->merge(render::RpAll);
				}
			);
		}

		renderGraph.addPass(rp);
	}

	m_lightSBuffer->unlock();
}

void WorldRendererForward::setupVisualPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t visualWriteTargetSetId,
	render::handle_t gbufferTargetSetId,
	render::handle_t ambientOcclusionTargetSetId,
	render::handle_t /*reflectionsTargetSetId*/,
	render::handle_t shadowMapAtlasTargetSetId
)
{
	T_PROFILER_SCOPE(L"WorldRendererForward setupVisualPass");
	const bool shadowsEnable = (bool)(m_shadowsQuality != Quality::Disabled);

	// Find first, non-local, probe.
	Ref< const ProbeComponent > probe;
	for (auto p : m_probes)
	{
		if (!p->getLocal() && p->getTexture() != nullptr)
		{
			probe = p;
			break;
		}
	}

	// Create render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Visual");
	rp->addInput(gbufferTargetSetId);

	if (ambientOcclusionTargetSetId != 0)
		rp->addInput(ambientOcclusionTargetSetId);

	//if (reflectionsTargetSetId != 0)
	//	rp->addInput(reflectionsTargetSetId);

	if (shadowsEnable)
		rp->addInput(shadowMapAtlasTargetSetId);

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 1.0f);
	rp->setOutput(visualWriteTargetSetId, clear, render::TfDepth, render::TfColor | render::TfDepth);

	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
#if defined(T_WORLD_FORWARD_USE_TILE_JOB)
			// Enure light clustering job has finished.
            Ref< Job > tileJob = m_tileJob;
            if (tileJob)
            {
                auto rb = renderContext->alloc< render::LambdaRenderBlock >();
                rb->lambda = [=](render::IRenderView*) {
                    tileJob->wait();
                };
                renderContext->enqueue(rb);
            }
			m_tileJob = nullptr;
#endif

			WorldBuildContext wc(
				m_entityRenderers,
				rootEntity,
				renderContext
			);

			auto gbufferTargetSet = renderGraph.getTargetSet(gbufferTargetSetId);
 			auto ambientOcclusionTargetSet = renderGraph.getTargetSet(ambientOcclusionTargetSetId);
			//auto reflectionsTargetSet = renderGraph.getTargetSet(reflectionsTargetSetId);
			auto shadowAtlasTargetSet = renderGraph.getTargetSet(shadowMapAtlasTargetSetId);

			float viewNearZ = worldRenderView.getViewFrustum().getNearZ();
			float viewFarZ = worldRenderView.getViewFrustum().getFarZ();
			float viewSliceScale = ClusterDimZ / std::log(viewFarZ / viewNearZ);
			float viewSliceBias = ClusterDimZ * std::log(viewNearZ) / std::log(viewFarZ / viewNearZ) - 0.001f;

			auto sharedParams = wc.getRenderContext()->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(wc.getRenderContext());
			sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
			sharedParams->setVectorParameter(s_handleViewDistance, Vector4(viewNearZ, viewFarZ, viewSliceScale, viewSliceBias));
			sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
			sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
			sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());

			if (m_irradianceGrid)
			{
				const auto size = m_irradianceGrid->getSize();
				sharedParams->setVectorParameter(s_handleIrradianceGridSize, Vector4((float)size[0] + 0.5f, (float)size[1] + 0.5f, (float)size[2] + 0.5f, 0.0f));
				sharedParams->setVectorParameter(s_handleIrradianceGridBoundsMin, m_irradianceGrid->getBoundingBox().mn);
				sharedParams->setVectorParameter(s_handleIrradianceGridBoundsMax, m_irradianceGrid->getBoundingBox().mx);
				sharedParams->setBufferViewParameter(s_handleIrradianceGridSBuffer, m_irradianceGrid->getBuffer()->getBufferView());
			}

			sharedParams->setBufferViewParameter(s_handleTileSBuffer, m_tileSBuffer->getBufferView());
			sharedParams->setBufferViewParameter(s_handleLightIndexSBuffer, m_lightIndexSBuffer->getBufferView());
			sharedParams->setBufferViewParameter(s_handleLightSBuffer, m_lightSBuffer->getBufferView());

			if (probe)
			{
				sharedParams->setFloatParameter(s_handleProbeIntensity, probe->getIntensity());
				sharedParams->setFloatParameter(s_handleProbeTextureMips, (float)probe->getTexture()->getMips());
				sharedParams->setTextureParameter(s_handleProbeTexture, probe->getTexture());
			}

			sharedParams->setVectorParameter(s_handleFogDistanceAndDensity, Vector4(m_settings.fogDistance, m_settings.fogDensity, 0.0f, 0.0f));
			sharedParams->setVectorParameter(s_handleFogColor, m_settings.fogColor);

			if (shadowAtlasTargetSet != nullptr)
				sharedParams->setTextureParameter(s_handleShadowMapAtlas, shadowAtlasTargetSet->getDepthTexture());

			sharedParams->setTextureParameter(s_handleDepthMap, gbufferTargetSet->getColorTexture(0));

			if (ambientOcclusionTargetSet != nullptr)
				sharedParams->setTextureParameter(s_handleOcclusionMap, ambientOcclusionTargetSet->getColorTexture(0));
			else
				sharedParams->setTextureParameter(s_handleOcclusionMap, m_whiteTexture);

			//if (reflectionsTargetSet != nullptr)
			//	sharedParams->setTextureParameter(s_handleReflectionMap, reflectionsTargetSet->getColorTexture(0));

			sharedParams->endParameters(wc.getRenderContext());

			WorldRenderPassForward defaultPass(
				s_techniqueForwardColor,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::PfLast,
				(bool)(m_irradianceGrid != nullptr),
				(bool)(shadowAtlasTargetSet != nullptr),
				(bool)(probe != nullptr) // (bool)(reflectionsTargetSet != nullptr)
			);

			T_ASSERT(!wc.getRenderContext()->havePendingDraws());

			for (auto gathered : m_gathered)
				gathered.entityRenderer->build(wc, worldRenderView, defaultPass, gathered.renderable);
	
			for (auto entityRenderer : m_entityRenderers->get())
				entityRenderer->build(wc, worldRenderView, defaultPass);

			wc.getRenderContext()->merge(render::RpAll);
		}
	);

	renderGraph.addPass(rp);
}

void WorldRendererForward::setupProcessPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId,
	render::handle_t gbufferTargetSetId,
	render::handle_t velocityTargetSetId,
	render::handle_t visualWriteTargetSetId,
	render::handle_t visualReadTargetSetId
) const
{
	T_PROFILER_SCOPE(L"WorldRendererForward setupProcessPass");
	render::ImageGraphView view;

	view.viewFrustum = worldRenderView.getViewFrustum();
	view.viewToLight = Matrix44::identity();
	view.view = worldRenderView.getView();
	view.projection = worldRenderView.getProjection();
	view.deltaTime = worldRenderView.getDeltaTime();
	view.time = worldRenderView.getTime();

	m_imageGraphContext->associateTextureTargetSet(s_handleInputColor, visualWriteTargetSetId, 0);
	m_imageGraphContext->associateTextureTargetSet(s_handleInputColorLast, visualReadTargetSetId, 0);
	m_imageGraphContext->associateTextureTargetSet(s_handleInputDepth, gbufferTargetSetId, 0);
	m_imageGraphContext->associateTextureTargetSet(s_handleInputNormal, gbufferTargetSetId, 1);
	m_imageGraphContext->associateTextureTargetSet(s_handleInputVelocity, velocityTargetSetId, 0);

	// Expose gamma and exposure.
	m_imageGraphContext->setFloatParameter(s_handleGamma, m_gamma);
	m_imageGraphContext->setFloatParameter(s_handleGammaInverse, 1.0f / m_gamma);
	m_imageGraphContext->setFloatParameter(s_handleExposure, std::pow(2.0f, m_settings.exposure));

	// Expose jitter; in texture space.
	Vector2 rc = jitter(m_count) / worldRenderView.getViewSize();
	Vector2 rp = jitter(m_count - 1) / worldRenderView.getViewSize();
	m_imageGraphContext->setVectorParameter(s_handleJitter, Vector4(rp.x, -rp.y, rc.x, -rc.y));

	StaticVector< render::ImageGraph*, 4 > processes;
	if (m_toneMap)
		processes.push_back(m_toneMap);
	if (m_antiAlias)
		processes.push_back(m_antiAlias);
	if (m_visual)
		processes.push_back(m_visual);
	if (m_gammaCorrection)
		processes.push_back(m_gammaCorrection);

	render::handle_t intermediateTargetSetId = 0;
	for (size_t i = 0; i < processes.size(); ++i)
	{
		auto process = processes[i];
		bool next = (bool)((i + 1) < processes.size());

		Ref< render::RenderPass > rp = new render::RenderPass(L"Process");

		if (next)
		{
			render::RenderGraphTargetSetDesc rgtd;
			rgtd.count = 1;
			rgtd.createDepthStencil = false;
			rgtd.usingPrimaryDepthStencil = false;
			rgtd.referenceWidthDenom = 1;
			rgtd.referenceHeightDenom = 1;
			rgtd.targets[0].colorFormat = render::TfR11G11B10F;
			intermediateTargetSetId = renderGraph.addTransientTargetSet(L"Process intermediate", rgtd, nullptr, outputTargetSetId);

			rp->setOutput(intermediateTargetSetId, render::TfColor, render::TfColor);
		}
		else
		{
			render::Clear cl;
			cl.mask = render::CfColor;
			cl.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
			rp->setOutput(outputTargetSetId, cl, render::TfDepth, render::TfColor | render::TfDepth);
		}

		process->addPasses(
			m_screenRenderer,
			renderGraph,
			rp,
			*m_imageGraphContext,
			view
		);

		if (next)
			m_imageGraphContext->associateTextureTargetSet(s_handleInputColor, intermediateTargetSetId, 0);

		renderGraph.addPass(rp);
	}
}

	}
}
