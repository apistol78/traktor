#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/ScreenRenderer.h"
#include "Render/StructBuffer.h"
#include "Render/StructElement.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Render/Image2/ImageGraphData.h"
#include "Resource/IResourceManager.h"
#include "World/Entity.h"
#include "World/IrradianceGrid.h"
#include "World/WorldBuildContext.h"
#include "World/WorldGatherContext.h"
#include "World/WorldHandles.h"
#include "World/WorldSetupContext.h"
#include "World/Forward/WorldRendererForward.h"
#include "World/Forward/WorldRenderPassForward.h"
#include "World/SMProj/UniformShadowProjection.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

#if defined(__ANDROID__) || defined(__IOS__)
const int32_t c_maxLightCount = 4;
#else
const int32_t c_maxLightCount = 1024;
#endif

const resource::Id< render::ImageGraph > c_ambientOcclusionLow(L"{416745F9-93C7-8D45-AE28-F2823DEE636A}");
const resource::Id< render::ImageGraph > c_ambientOcclusionMedium(L"{5A3B0260-32F9-B343-BBA4-88BD932F917A}");
const resource::Id< render::ImageGraph > c_ambientOcclusionHigh(L"{45F9CD9F-C700-9942-BB36-443629C88748}");
const resource::Id< render::ImageGraph > c_ambientOcclusionUltra(L"{302E57C8-711D-094F-A764-75F76553E81B}");
const resource::Id< render::ImageGraph > c_antiAliasLow(L"{71D385F1-8364-C849-927F-5F1249F5DF92}");
const resource::Id< render::ImageGraph > c_antiAliasMedium(L"{D03B9566-EFA3-7A43-B3AD-F59DB34DEE96}");
const resource::Id< render::ImageGraph > c_antiAliasHigh(L"{C0316981-FA73-A34E-8135-1F596425688F}");
//const resource::Id< render::ImageGraph > c_antiAliasUltra(L"{88E329C8-A2F3-7443-B73E-4E85C6ECACBE}");
const resource::Id< render::ImageGraph > c_gammaCorrection(L"{B1E8367D-91DD-D648-A44F-B86492169771}");
const resource::Id< render::ImageGraph > c_toneMapFixed(L"{1F20DAB5-22EB-B84C-92B0-71E94C1CE261}");
const resource::Id< render::ImageGraph > c_toneMapAdaptive(L"{BE19DE90-E010-A74D-AA3B-87FAC2A56946}");

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
		//return c_antiAliasUltra;
		return c_antiAliasHigh;		// Since forward path doesn't include velocity buffer then we cannot use TAA.
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
	m_shadowsQuality = desc.quality.shadows;
	m_ambientOcclusionQuality = desc.quality.ambientOcclusion;
	m_antiAliasQuality = desc.quality.antiAlias;
	m_gamma = desc.gamma;
	m_sharedDepthStencil = desc.sharedDepthStencil;

	// Allocate frames, one for each queued frame.
	m_frames.resize(desc.frameCount);

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
	if (
		m_settings.linearLighting &&
		std::abs(m_gamma - 1.0f) > FUZZY_EPSILON
	)
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

	// Allocate light lists.
	for (auto& frame : m_frames)
	{
		AlignedVector< render::StructElement > lightShaderDataStruct;
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, typeRangeRadius)));
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, position)));
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, direction)));
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, color)));
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, viewToLight0)));
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, viewToLight1)));
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, viewToLight2)));
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, viewToLight3)));
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, atlasTransform)));
		T_FATAL_ASSERT(sizeof(LightShaderData) == render::getStructSize(lightShaderDataStruct));

		frame.lightSBuffer = renderSystem->createStructBuffer(
			lightShaderDataStruct,
			render::getStructSize(lightShaderDataStruct) * c_maxLightCount
		);
		if (!frame.lightSBuffer)
			return false;

		frame.lightSBufferMemory = frame.lightSBuffer->lock();
		if (!frame.lightSBufferMemory)
			return false;
	}

	// Create irradiance grid.
	if (!m_settings.irradianceGrid.isNull())
	{
		if (!resourceManager->bind(m_settings.irradianceGrid, m_irradianceGrid))
			return false;
	}

	// Determine slice distances.
	const auto& shadowSettings = m_settings.shadowSettings[(int32_t)m_shadowsQuality];
	for (int32_t i = 0; i < shadowSettings.cascadingSlices; ++i)
	{
		float ii = float(i) / shadowSettings.cascadingSlices;
		float log = powf(ii, shadowSettings.cascadingLambda);
		m_slicePositions[i] = lerp(m_settings.viewNearZ, shadowSettings.farZ, log);
	}
	m_slicePositions[shadowSettings.cascadingSlices] = shadowSettings.farZ;

	m_entityRenderers = desc.entityRenderers;

	// Create screen renderer.
	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	// Misc resources.
	m_blackTexture = create1x1Texture(renderSystem, 0x00000000);
	m_whiteTexture = create1x1Texture(renderSystem, 0xffffffff);

	m_count = 0;
	return true;
}

void WorldRendererForward::destroy()
{
	for (auto& frame : m_frames)
		safeDestroy(frame.lightSBuffer);
	m_frames.clear();

	safeDestroy(m_screenRenderer);
	safeDestroy(m_blackTexture);
	safeDestroy(m_whiteTexture);

	m_irradianceGrid.clear();
}

void WorldRendererForward::setup(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId
)
{
	int32_t frame = m_count % (int32_t)m_frames.size();

	// Gather active lights.
	m_lights.resize(0);
	WorldGatherContext(m_entityRenderers, rootEntity).gather(rootEntity, m_lights);
	if (m_lights.size() > c_maxLightCount)
		m_lights.resize(c_maxLightCount);

	// Begun writing light shader data; written both in setup and build.
	LightShaderData* lightShaderData = (LightShaderData*)m_frames[frame].lightSBufferMemory;

	// Add additional passes by entity renderers.
	{
		WorldSetupContext context(m_entityRenderers, rootEntity, renderGraph);
		context.setup(worldRenderView, rootEntity);
		context.flush();
	}

	// Add passes to render graph.
	auto gbufferTargetSetId = setupGBufferPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		outputTargetSetId
	);

	auto ambientOcclusionTargetSetId = setupAmbientOcclusionPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		outputTargetSetId,
		gbufferTargetSetId
	);

	render::handle_t shadowMapCascadeTargetSetId = 0;
	render::handle_t shadowMapAtlasTargetSetId = 0;
	setupLightPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		outputTargetSetId,
		frame,
		lightShaderData,
		shadowMapCascadeTargetSetId,
		shadowMapAtlasTargetSetId
	);

	auto visualTargetSetId = setupVisualPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		outputTargetSetId,
		gbufferTargetSetId,
		ambientOcclusionTargetSetId,
		shadowMapCascadeTargetSetId,
		shadowMapAtlasTargetSetId,
		frame
	);

	setupProcessPass(
		worldRenderView,
		rootEntity,
		renderGraph,
		outputTargetSetId,
		gbufferTargetSetId,
		visualTargetSetId
	);

	m_count++;
}

render::handle_t WorldRendererForward::setupGBufferPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId
) const
{
	const float clearZ = m_settings.viewFarZ;

	// Add GBuffer target set.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 2;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = (m_sharedDepthStencil == nullptr) ? true : false;
	rgtd.targets[0].colorFormat = render::TfR16F;		// Depth (R)
	rgtd.targets[1].colorFormat = render::TfR16G16F;	// Normals (RG)
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	auto gbufferTargetSetId = renderGraph.addTransientTargetSet(L"GBuffer", rgtd, m_sharedDepthStencil, outputTargetSetId);

	// Add GBuffer render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"GBuffer");
	
	render::Clear clear;
	clear.mask = render::CfColor | render::CfDepth | render::CfStencil;
	clear.colors[0] = Color4f(clearZ, clearZ, clearZ, clearZ);
	clear.colors[1] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
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
			sharedParams->endParameters(renderContext);

			WorldRenderPassForward pass(
				s_techniqueForwardGBufferWrite,
				sharedParams,
				IWorldRenderPass::PfFirst,
				worldRenderView.getView(),
				nullptr,
				nullptr,
				nullptr
			);

			T_ASSERT(!renderContext->havePendingDraws());
			wc.build(worldRenderView, pass, rootEntity);
			wc.flush(worldRenderView, pass);
			renderContext->merge(render::RpAll);		
		}
	);

	renderGraph.addPass(rp);
	return gbufferTargetSetId;
}

render::handle_t WorldRendererForward::setupAmbientOcclusionPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId,
	render::handle_t gbufferTargetSetId
) const
{
	if (m_ambientOcclusion == nullptr)
		return 0;

	// Add ambient occlusion target set.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = false;
	rgtd.targets[0].colorFormat = render::TfR8;			// Ambient occlusion (R)
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	auto ambientOcclusionTargetSetId = renderGraph.addTransientTargetSet(L"Ambient occlusion", rgtd, m_sharedDepthStencil, outputTargetSetId);

	// Add ambient occlusion render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Ambient occlusion");

	render::ImageGraphParams ipd;
	ipd.viewFrustum = worldRenderView.getViewFrustum();
	ipd.view = worldRenderView.getView();
	ipd.projection = worldRenderView.getProjection();

	render::ImageGraphContext cx(m_screenRenderer);
	cx.associateTextureTargetSet(s_handleInputDepth, gbufferTargetSetId, 0);
	cx.associateTextureTargetSet(s_handleInputNormal, gbufferTargetSetId, 1);
	cx.setParams(ipd);

	m_ambientOcclusion->addPasses(renderGraph, rp, cx);

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	rp->setOutput(ambientOcclusionTargetSetId, clear, render::TfNone, render::TfColor);

	renderGraph.addPass(rp);
	return ambientOcclusionTargetSetId;
}

void WorldRendererForward::setupLightPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId,
	int32_t frame,
	LightShaderData* lightShaderData,
	render::handle_t& outShadowMapCascadeTargetSetId,
	render::handle_t& outShadowMapAtlasTargetSetId
) const
{
	const UniformShadowProjection shadowProjection(1024);
	const auto& shadowSettings = m_settings.shadowSettings[(int32_t)m_shadowsQuality];
	const bool shadowsEnable = (bool)(m_shadowsQuality != Quality::Disabled);

	Matrix44 view = worldRenderView.getView();
	Matrix44 viewInverse = worldRenderView.getView().inverse();
	Frustum viewFrustum = worldRenderView.getViewFrustum();
	
	// Find cascade shadow light.
	int32_t lightCascadeIndex = -1;
	if (shadowsEnable)
	{
		for (int32_t i = 0; i < (int32_t)m_lights.size(); ++i)
		{
			const auto& light = m_lights[i];
			if (light.castShadow && light.type == LtDirectional)
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
		for (int32_t i = 0; i < (int32_t)m_lights.size(); ++i)
		{
			const auto& light = m_lights[i];
			if (light.castShadow && light.type == LtSpot)
				lightAtlasIndices.push_back(i);
		}
	}

	// Write all lights to sbuffer; without shadow map information.
	for (int32_t i = 0; i < (int32_t)m_lights.size(); ++i)
	{
		const auto& light = m_lights[i];
		auto* lsd = &lightShaderData[i];

		lsd->typeRangeRadius[0] = (float)light.type;
		lsd->typeRangeRadius[1] = light.range;
		lsd->typeRangeRadius[2] = light.radius / 2.0f;
		lsd->typeRangeRadius[3] = 0.0f;

		(view * light.position.xyz1()).storeUnaligned(lsd->position);
		(view * light.direction.xyz0()).storeUnaligned(lsd->direction);
		light.color.storeUnaligned(lsd->color);

		Vector4::zero().storeUnaligned(lsd->viewToLight0);
		Vector4::zero().storeUnaligned(lsd->viewToLight1);
		Vector4::zero().storeUnaligned(lsd->viewToLight2);
		Vector4::zero().storeUnaligned(lsd->viewToLight3);
	}

	// If shadow casting directional light found add cascade shadow map pass
	// and update light sbuffer.
	if (shadowsEnable)
	{
		// Add cascading shadow map target.
		render::RenderGraphTargetSetDesc rgtd;
		rgtd.count = 0;
		rgtd.width = shadowSettings.resolution;
		rgtd.height = shadowSettings.cascadingSlices * shadowSettings.resolution;
		rgtd.createDepthStencil = true;
		rgtd.usingPrimaryDepthStencil = false;
		rgtd.usingDepthStencilAsTexture = true;
		rgtd.ignoreStencil = true;
		outShadowMapCascadeTargetSetId = renderGraph.addTransientTargetSet(L"Shadow map cascade", rgtd);

		// Add cascading shadow map render pass.
		Ref< render::RenderPass > rp = new render::RenderPass(L"Shadow cascade");

		render::Clear clear;
		clear.mask = render::CfDepth;
		clear.depth = 1.0f;
		rp->setOutput(outShadowMapCascadeTargetSetId, clear, render::TfNone, render::TfDepth);

		if (lightCascadeIndex >= 0)
			rp->addBuild(
				[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
				{
					WorldBuildContext wc(
						m_entityRenderers,
						rootEntity,
						renderContext
					);

					const auto& light = m_lights[lightCascadeIndex];
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
							light.position,
							light.direction,
							sliceViewFrustum,
							shadowSettings.farZ,
							shadowSettings.quantizeProjection,
							shadowLightView,
							shadowLightProjection,
							shadowFrustum
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
							0,
							slice * 1024,
							1024,
							1024,
							0.0f,
							1.0f
						);
						renderContext->enqueue(svrb);	

						// Render entities into shadow map.
						auto sharedParams = renderContext->alloc< render::ProgramParameters >();
						sharedParams->beginParameters(renderContext);
						sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
						sharedParams->setMatrixParameter(s_handleView, shadowLightView);
						sharedParams->setMatrixParameter(s_handleViewInverse, shadowLightView.inverse());
						sharedParams->setMatrixParameter(s_handleProjection, shadowLightProjection);
						sharedParams->endParameters(renderContext);

						WorldRenderPassForward shadowPass(
							s_techniqueShadow,
							sharedParams,
							IWorldRenderPass::PfNone,
							shadowRenderView.getView(),
							nullptr,
							nullptr,
							nullptr
						);

						T_ASSERT(!renderContext->havePendingDraws());
						wc.build(shadowRenderView, shadowPass, rootEntity);
						wc.flush(shadowRenderView, shadowPass);
						renderContext->merge(render::RpAll);

						// Write transposed matrix to shaders as shaders have row-major order.
						Matrix44 viewToLightSpace = shadowLightProjection * shadowLightView * viewInverse;
						Matrix44 vls = viewToLightSpace.transpose();
						vls.axisX().storeUnaligned(lsd->viewToLight0);
						vls.axisY().storeUnaligned(lsd->viewToLight1);
						vls.axisZ().storeUnaligned(lsd->viewToLight2);
						vls.translation().storeUnaligned(lsd->viewToLight3);

						// Write slice coordinates to shaders.
						Vector4(
							0.0f,
							float(slice) / shadowSettings.cascadingSlices,
							1.0f,
							1.0f / shadowSettings.cascadingSlices
						).storeUnaligned(lsd->atlasTransform);
					}
				}
			);

		renderGraph.addPass(rp);
	}

	if (shadowsEnable)
	{
		// Add atlas shadow map target.
		render::RenderGraphTargetSetDesc rgtd;
		rgtd.count = 0;
		rgtd.width =
		rgtd.height = 4096;
		rgtd.createDepthStencil = true;
		rgtd.usingPrimaryDepthStencil = false;
		rgtd.usingDepthStencilAsTexture = true;
		rgtd.ignoreStencil = true;
		outShadowMapAtlasTargetSetId = renderGraph.addTransientTargetSet(L"Shadow map atlas", rgtd);

		// Add atlas shadow map render pass.
		int32_t atlasIndex = 0;
		for (int32_t lightAtlasIndex : lightAtlasIndices)
		{
			Ref< render::RenderPass > rp = new render::RenderPass(L"Shadow atlas");

			render::Clear clear;
			clear.mask = render::CfDepth;
			clear.depth = 1.0f;
			rp->setOutput(outShadowMapAtlasTargetSetId, clear, render::TfDepth, render::TfDepth);
			
			if (!lightAtlasIndices.empty())
				rp->addBuild(
					[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
					{
						WorldBuildContext wc(
							m_entityRenderers,
							rootEntity,
							renderContext
						);

						const auto& light = m_lights[lightAtlasIndex];
						auto* lsd = &lightShaderData[lightAtlasIndex];

						// Calculate shadow map projection.
						Matrix44 shadowLightView;
						Matrix44 shadowLightProjection;
						Frustum shadowFrustum;

						shadowFrustum.buildPerspective(
							light.radius,
							1.0f,
							0.1f,
							light.range
						);

						shadowLightProjection = perspectiveLh(light.radius, 1.0f, 0.1f, light.range);

						Vector4 lightAxisX, lightAxisY, lightAxisZ;
						lightAxisZ = -light.direction.xyz0().normalized();
						lightAxisX = cross(viewInverse.axisZ(), lightAxisZ).normalized();
						lightAxisY = cross(lightAxisX, lightAxisZ).normalized();

						shadowLightView = Matrix44(
							lightAxisX,
							lightAxisY,
							lightAxisZ,
							light.position.xyz1()
						);
						shadowLightView = shadowLightView.inverse();

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
							(atlasIndex & 3) * 1024,
							(atlasIndex / 4) * 1024,
							1024,
							1024,
							0.0f,
							1.0f
						);
						renderContext->enqueue(svrb);	

						// Render entities into shadow map.
						auto sharedParams = renderContext->alloc< render::ProgramParameters >();
						sharedParams->beginParameters(renderContext);
						sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
						sharedParams->setMatrixParameter(s_handleView, shadowLightView);
						sharedParams->setMatrixParameter(s_handleViewInverse, shadowLightView.inverse());
						sharedParams->setMatrixParameter(s_handleProjection, shadowLightProjection);
						sharedParams->endParameters(renderContext);

						WorldRenderPassForward shadowPass(
							s_techniqueShadow,
							sharedParams,
							IWorldRenderPass::PfNone,
							shadowRenderView.getView(),
							nullptr,
							nullptr,
							nullptr
						);

						T_ASSERT(!renderContext->havePendingDraws());
						wc.build(shadowRenderView, shadowPass, rootEntity);
						wc.flush(shadowRenderView, shadowPass);
						renderContext->merge(render::RpAll);

						// Write transposed matrix to shaders as shaders have row-major order.
						Matrix44 viewToLightSpace = shadowLightProjection * shadowLightView * viewInverse;
						Matrix44 vls = viewToLightSpace.transpose();
						vls.axisX().storeUnaligned(lsd->viewToLight0);
						vls.axisY().storeUnaligned(lsd->viewToLight1);
						vls.axisZ().storeUnaligned(lsd->viewToLight2);
						vls.translation().storeUnaligned(lsd->viewToLight3);

						// Write atlas coordinates to shaders.
						Vector4(
							(atlasIndex & 3) / 4.0f,
							(atlasIndex / 4) / 4.0f,
							1.0f / 4.0f,
							1.0f / 4.0f
						).storeUnaligned(lsd->atlasTransform);					
					}
				);

			renderGraph.addPass(rp);

			++atlasIndex;
		}
	}
}

render::handle_t WorldRendererForward::setupVisualPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId,
	render::handle_t gbufferTargetSetId,
	render::handle_t ambientOcclusionTargetSetId,
	render::handle_t shadowMapCascadeTargetSetId,
	render::handle_t shadowMapAtlasTargetSetId,
	int32_t frame
) const
{
	const bool shadowsEnable = (bool)(m_shadowsQuality != Quality::Disabled);
	int32_t lightCount = (int32_t)m_lights.size();

	// Add visual[0] target set.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = (m_sharedDepthStencil == nullptr) ? true : false;
	rgtd.targets[0].colorFormat = render::TfR11G11B10F;
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	auto visualTargetSetId = renderGraph.addTransientTargetSet(L"Visual", rgtd, m_sharedDepthStencil, outputTargetSetId);

	// Create render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Visual");
	rp->addInput(gbufferTargetSetId);

	if (ambientOcclusionTargetSetId != 0)
		rp->addInput(ambientOcclusionTargetSetId);

	if (shadowsEnable)
	{
		rp->addInput(shadowMapCascadeTargetSetId);
		rp->addInput(shadowMapAtlasTargetSetId);
	}

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 1.0f);
	rp->setOutput(visualTargetSetId, clear, render::TfDepth, render::TfColor | render::TfDepth);

	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			WorldBuildContext wc(
				m_entityRenderers,
				rootEntity,
				renderContext
			);

			auto gbufferTargetSet = renderGraph.getTargetSet(gbufferTargetSetId);
 			auto ambientOcclusionTargetSet = (ambientOcclusionTargetSetId != 0) ? renderGraph.getTargetSet(ambientOcclusionTargetSetId) : nullptr;
			auto shadowCascadeTargetSet = renderGraph.getTargetSet(shadowMapCascadeTargetSetId);
			auto shadowAtlasTargetSet = renderGraph.getTargetSet(shadowMapAtlasTargetSetId);

			auto sharedParams = wc.getRenderContext()->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(wc.getRenderContext());
			sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
			sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());

			if (m_irradianceGrid)
			{
				const auto size = m_irradianceGrid->getSize();
				sharedParams->setVectorParameter(s_handleIrradianceGridSize, Vector4((float)size[0], (float)size[1], (float)size[2], 0.0f));
				sharedParams->setVectorParameter(s_handleIrradianceGridBoundsMin, m_irradianceGrid->getBoundingBox().mn);
				sharedParams->setVectorParameter(s_handleIrradianceGridBoundsMax, m_irradianceGrid->getBoundingBox().mx);
				sharedParams->setStructBufferParameter(s_handleIrradianceGridSBuffer, m_irradianceGrid->getBuffer());
			}

			sharedParams->endParameters(wc.getRenderContext());

			WorldRenderPassForward defaultPass(
				s_techniqueForwardColor,
				sharedParams,
				IWorldRenderPass::PfLast,
				worldRenderView.getView(),
				m_frames[frame].lightSBuffer,
				lightCount,
				(bool)(m_irradianceGrid != nullptr),
				m_settings.fog,
				m_settings.fogDistanceY,
				m_settings.fogDistanceZ,
				m_settings.fogDensityY,
				m_settings.fogDensityZ,
				m_settings.fogColor,
				nullptr,
				gbufferTargetSet->getColorTexture(0),
				(ambientOcclusionTargetSet != nullptr) ? ambientOcclusionTargetSet->getColorTexture(0) : m_whiteTexture.ptr(),
				(shadowCascadeTargetSet != nullptr) ? shadowCascadeTargetSet->getDepthTexture() : nullptr,
				(shadowAtlasTargetSet != nullptr) ? shadowAtlasTargetSet->getDepthTexture() : nullptr
			);

			T_ASSERT(!wc.getRenderContext()->havePendingDraws());
			wc.build(worldRenderView, defaultPass, rootEntity);
			wc.flush(worldRenderView, defaultPass);
			wc.getRenderContext()->merge(render::RpAll);
		}
	);

	renderGraph.addPass(rp);
	return visualTargetSetId;
}

void WorldRendererForward::setupProcessPass(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId,
	render::handle_t gbufferTargetSetId,
	render::handle_t visualTargetSetId
) const
{
	render::ImageGraphParams ipd;
	ipd.viewFrustum = worldRenderView.getViewFrustum();
	ipd.viewToLight = Matrix44::identity();
	ipd.view = worldRenderView.getView();
	ipd.projection = worldRenderView.getProjection();
	ipd.deltaTime = 1.0f / 60.0f;				

	render::ImageGraphContext cx(m_screenRenderer);
	cx.associateTextureTargetSet(s_handleInputColor, visualTargetSetId, 0);
	cx.associateTextureTargetSet(s_handleInputDepth, gbufferTargetSetId, 0);
	cx.associateTextureTargetSet(s_handleInputNormal, gbufferTargetSetId, 1);
	cx.setFloatParameter(s_handleGamma, m_gamma);
	cx.setFloatParameter(s_handleGammaInverse, 1.0f / m_gamma);
	cx.setFloatParameter(s_handleExposure, m_settings.exposure);
	cx.setParams(ipd);

	StaticVector< render::ImageGraph*, 4 > processes;
	if (m_toneMap)
		processes.push_back(m_toneMap);
	if (m_visual)
		processes.push_back(m_visual);
	if (m_gammaCorrection)
		processes.push_back(m_gammaCorrection);
	if (m_antiAlias)
		processes.push_back(m_antiAlias);

	render::handle_t intermediateTargetSetId = 0;
	for (size_t i = 0; i < processes.size(); ++i)
	{
		auto process = processes[i];
		bool next = (bool)((i + 1) < processes.size());

		Ref< render::RenderPass > rp = new render::RenderPass(L"Process");

		if (next)
		{
			render::RenderGraphTargetSetDesc rgtd = {};
			rgtd.count = 1;
			rgtd.createDepthStencil = false;
			rgtd.usingPrimaryDepthStencil = false;
#if defined(__ANDROID__)
			rgtd.targets[0].colorFormat = render::TfR8G8B8A8;
#else
			rgtd.targets[0].colorFormat = render::TfR11G11B10F;
#endif
			rgtd.referenceWidthDenom = 1;
			rgtd.referenceHeightDenom = 1;
			intermediateTargetSetId = renderGraph.addTransientTargetSet(L"Process intermediate", rgtd, nullptr, outputTargetSetId);

			rp->setOutput(intermediateTargetSetId, render::TfColor, render::TfColor);
		}
		else
		{
			render::Clear cl;
			cl.mask = render::CfColor;
			cl.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
			rp->setOutput(outputTargetSetId, cl, render::TfNone, render::TfColor);
		}

		process->addPasses(renderGraph, rp, cx);

		if (next)
			cx.associateTextureTargetSet(s_handleInputColor, intermediateTargetSetId, 0);

		renderGraph.addPass(rp);
	}
}

	}
}
