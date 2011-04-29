#include "Core/Log/Log.h"
#include "Core/Math/Random.h"
#include "Core/Math/Format.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/ISimpleTexture.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "World/WorldRenderView.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldContext.h"
#include "World/Entity/IEntityRenderer.h"
#include "World/Entity/Entity.h"
#include "World/PostProcess/PostProcess.h"
#include "World/PostProcess/PostProcessSettings.h"
#include "World/PreLit/LightRenderer.h"
#include "World/PreLit/WorldRendererPreLit.h"
#include "World/PreLit/WorldRenderPassPreLit.h"
#include "World/SMProj/BoxSMProj.h"
#include "World/SMProj/LiSPSMProj.h"
#include "World/SMProj/TSMProj.h"
#include "World/SMProj/UniformSMProj.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

const Guid c_shadowMaskProject(L"{F751F3EB-33D2-9247-9E3F-54B1A0E3522C}");
const Guid c_shadowMaskFilterNone(L"{19222311-363F-CB45-86E5-34D376CDA8AD}");
const Guid c_shadowMaskFilterLow(L"{7D4D38B9-1E43-8046-B1A4-705CFEF9B8EB}");
const Guid c_shadowMaskFilterMedium(L"{57FD53AF-547A-9F46-8C94-B4D24EFB63BC}");
const Guid c_shadowMaskFilterHigh(L"{FABC4017-4D65-604D-B9AB-9FC03FE3CE43}");
const Guid c_shadowMaskFilterHighest(L"{5AFC153E-6FCE-3142-9E1B-DD3722DA447F}");

const static float c_interocularDistance = 6.5f;
const static float c_distortionValue = 0.8f;
const static float c_screenPlaneDistance = 13.0f;

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRendererPreLit", 0, WorldRendererPreLit, IWorldRenderer)

render::handle_t WorldRendererPreLit::ms_techniquePreLitColor = 0;
render::handle_t WorldRendererPreLit::ms_techniqueDepth = 0;
render::handle_t WorldRendererPreLit::ms_techniqueNormal = 0;
render::handle_t WorldRendererPreLit::ms_techniqueShadow = 0;
render::handle_t WorldRendererPreLit::ms_handleProjection = 0;

WorldRendererPreLit::WorldRendererPreLit()
:	m_count(0)
{
	// Techniques
	ms_techniquePreLitColor = render::getParameterHandle(L"World_PreLitColor");
	ms_techniqueDepth = render::getParameterHandle(L"World_DepthWrite");
	ms_techniqueNormal = render::getParameterHandle(L"World_NormalWrite");
	ms_techniqueShadow = render::getParameterHandle(L"World_ShadowWrite");

	// Global parameters.
	ms_handleProjection = render::getParameterHandle(L"Projection");
}

bool WorldRendererPreLit::create(
	const WorldRenderSettings& settings,
	WorldEntityRenderers* entityRenderers,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	render::IRenderView* renderView,
	uint32_t multiSample,
	uint32_t frameCount
)
{
	T_ASSERT_M (renderView, L"Render view required");

	m_settings = settings;
	m_renderView = renderView;
	m_frames.resize(frameCount);

	int32_t width = renderView->getWidth();
	int32_t height = renderView->getHeight();

	// Create "depth map" target.
	{
		render::RenderTargetSetCreateDesc desc;

		desc.count = 1;
		desc.width = width;
		desc.height = height;
		desc.multiSample = multiSample;
		desc.createDepthStencil = false;
		desc.usingPrimaryDepthStencil = true;
		desc.preferTiled = true;
		desc.targets[0].format = render::TfR8G8B8A8;

		m_depthTargetSet = renderSystem->createRenderTargetSet(desc);

		if (!m_depthTargetSet && multiSample > 0)
		{
			desc.multiSample = 0;
			desc.createDepthStencil = true;
			desc.usingPrimaryDepthStencil = false;

			m_depthTargetSet = renderSystem->createRenderTargetSet(desc);
			if (m_depthTargetSet)
				log::warning << L"MSAA depth render target unsupported; may cause poor performance" << Endl;
		}

		if (!m_depthTargetSet)
		{
			log::error << L"Unable to create depth render target" << Endl;
			return false;
		}
	}

	// Create "normal map" targets.
	{
		render::RenderTargetSetCreateDesc desc;

		desc.count = 1;
		desc.width = width;
		desc.height = height;
		desc.multiSample = multiSample;
		desc.createDepthStencil = false;
		desc.usingPrimaryDepthStencil = true;
		desc.preferTiled = true;
		desc.targets[0].format = render::TfR8G8B8A8;

		m_normalTargetSet = renderSystem->createRenderTargetSet(desc);

		if (!m_normalTargetSet && multiSample > 0)
		{
			desc.multiSample = 0;
			desc.createDepthStencil = true;
			desc.usingPrimaryDepthStencil = false;

			m_normalTargetSet = renderSystem->createRenderTargetSet(desc);
			if (m_normalTargetSet)
				log::warning << L"MSAA normal render target unsupported; may cause poor performance" << Endl;
		}

		if (!m_normalTargetSet)
		{
			log::error << L"Unable to create normal render target" << Endl;
			return false;
		}
	}

	// Create "shadow map" targets.
	if (m_settings.shadowsEnabled)
	{
		render::RenderTargetSetCreateDesc desc;

		// Create shadow map target.
		desc.count = 1;
		desc.width =
		desc.height = m_settings.shadowMapResolution;
		desc.multiSample = 0;
		desc.createDepthStencil = true;
		desc.usingPrimaryDepthStencil = false;
		desc.preferTiled = true;
		desc.targets[0].format = render::TfR8G8B8A8;

		switch (m_settings.shadowsQuality)
		{
		case WorldRenderSettings::SqLow:
			desc.width /= 4;
			desc.height /= 4;
			break;

		case WorldRenderSettings::SqMedium:
			desc.width /= 2;
			desc.height /= 2;
			break;
		}

		m_shadowTargetSet = renderSystem->createRenderTargetSet(desc);

		// Determine shadow mask size; high quality is same as entire screen.
		if (
			m_settings.shadowsQuality == WorldRenderSettings::SqNoFilter ||
			m_settings.shadowsQuality == WorldRenderSettings::SqHigh ||
			m_settings.shadowsQuality == WorldRenderSettings::SqHighest
		)
		{
			desc.width = width;
			desc.height = height;
		}
		else
		{
			desc.width = width / 2;
			desc.height = height / 2;
		}

		// Create shadow mask target.
		desc.count = 1;
		desc.multiSample = 0;
		desc.createDepthStencil = false;
		desc.usingPrimaryDepthStencil = false;
		desc.targets[0].format = render::TfR8;
		desc.preferTiled = true;
		m_shadowMaskProjectTargetSet = renderSystem->createRenderTargetSet(desc);

		// Create filtered shadow mask target.
		desc.count = 1;
		desc.multiSample = 0;
		desc.createDepthStencil = false;
		desc.usingPrimaryDepthStencil = false;
		desc.targets[0].format = render::TfR8;
		desc.preferTiled = true;
		m_shadowMaskFilterTargetSet = renderSystem->createRenderTargetSet(desc);

		if (
			m_shadowTargetSet &&
			m_shadowMaskProjectTargetSet &&
			m_shadowMaskFilterTargetSet
		)
		{
			resource::Proxy< PostProcessSettings > shadowMaskProject;
			resource::Proxy< PostProcessSettings > shadowMaskFilter;

			shadowMaskProject = c_shadowMaskProject;

			switch (m_settings.shadowsQuality)
			{
			case WorldRenderSettings::SqNoFilter:
				shadowMaskFilter = c_shadowMaskFilterNone;
				break;
			case WorldRenderSettings::SqLow:
				shadowMaskFilter = c_shadowMaskFilterLow;
				break;
			case WorldRenderSettings::SqMedium:
				shadowMaskFilter = c_shadowMaskFilterMedium;
				break;
			case WorldRenderSettings::SqHigh:
				shadowMaskFilter = c_shadowMaskFilterHigh;
				break;
			case WorldRenderSettings::SqHighest:
				shadowMaskFilter = c_shadowMaskFilterHighest;
				break;
			}

			resourceManager->bind(shadowMaskProject);
			resourceManager->bind(shadowMaskFilter);

			m_shadowMaskProject = new PostProcess();
			if (!m_shadowMaskProject->create(
				shadowMaskProject,
				resourceManager,
				renderSystem,
				desc.width,
				desc.height
			))
			{
				log::warning << L"Unable to create shadow project process; shadows disabled" << Endl;
				m_settings.shadowsEnabled = false;
			}

			m_shadowMaskFilter = new PostProcess();
			if (!m_shadowMaskFilter->create(
				shadowMaskFilter,
				resourceManager,
				renderSystem,
				desc.width,
				desc.height
			))
			{
				log::warning << L"Unable to create shadow filter process; shadows disabled" << Endl;
				m_settings.shadowsEnabled = false;
			}
		}
		else
		{
			log::warning << L"Unable to create shadow render targets; shadows disabled" << Endl;
			m_settings.shadowsEnabled = false;
		}

		// Ensure targets are destroyed if something went wrong in setup.
		if (!m_settings.shadowsEnabled)
		{
			safeDestroy(m_shadowTargetSet);
			safeDestroy(m_shadowMaskProjectTargetSet);
			safeDestroy(m_shadowMaskFilterTargetSet);
		}
	}

	// Create light map target.
	{
		render::RenderTargetSetCreateDesc desc;

		desc.count = 1;
		desc.width = width;
		desc.height = height;
		desc.multiSample = multiSample;
		desc.createDepthStencil = false;
		desc.usingPrimaryDepthStencil = true;
		desc.preferTiled = true;
		desc.targets[0].format = render::TfR16G16B16A16F;

		m_lightMapTargetSet = renderSystem->createRenderTargetSet(desc);
		if (!m_lightMapTargetSet)
		{
			log::error << L"Unable to create light map render target" << Endl;
			return false;
		}
	}

	// Allocate "depth" context.
	{
		for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
			i->depth = new WorldContext(entityRenderers);
	}

	// Allocate "normal" context.
	{
		for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
			i->normal = new WorldContext(entityRenderers);
	}

	// Allocate "shadow" contexts.
	if (m_settings.shadowsEnabled)
	{
		for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
		{
			for (int32_t j = 0; j < m_settings.shadowCascadingSlices; ++j)
			{
				for (int32_t k = 0; k < MaxLightCount; ++k)
					i->slice[j].shadow[k] = new WorldContext(entityRenderers);
			}
		}
	}

	// Allocate "visual" contexts.
	for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
		i->visual = new WorldContext(entityRenderers);

	// Allocate "global" parameter context; as it's reset for each render
	// call this can be fairly small.
	m_globalContext = new render::RenderContext(4096);

	// Create light primitive renderer.
	m_lightRenderer = new LightRenderer();
	if (!m_lightRenderer->create(resourceManager, renderSystem))
	{
		log::error << L"Unable to create light primitive renderer" << Endl;
		return false;
	}

	// Determine slice distances.
	for (int32_t i = 0; i < m_settings.shadowCascadingSlices; ++i)
	{
		float ii = float(i) / m_settings.shadowCascadingSlices;
		float log = m_settings.viewNearZ * powf(m_settings.shadowFarZ / m_settings.viewNearZ, ii);
		float uni = m_settings.viewNearZ + (m_settings.shadowFarZ - m_settings.viewNearZ) * ii;
		m_slicePositions[i] = log * m_settings.shadowCascadingLambda + uni * (1.0f - m_settings.shadowCascadingLambda);
	}
	m_slicePositions[m_settings.shadowCascadingSlices] = m_settings.shadowFarZ;

	m_count = 0;
	return true;
}

void WorldRendererPreLit::destroy()
{
	safeDestroy(m_lightRenderer);

	for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
	{
		for (int32_t j = 0; j < MaxSliceCount; ++j)
		{
			for (int32_t k = 0; k < MaxLightCount; ++k)
				i->slice[j].shadow[k] = 0;
		}

		i->visual = 0;
		i->depth = 0;
		i->normal = 0;
	}

	safeDestroy(m_shadowMaskFilter);
	safeDestroy(m_shadowMaskProject);
	safeDestroy(m_shadowMaskFilterTargetSet);
	safeDestroy(m_shadowMaskProjectTargetSet);
	safeDestroy(m_shadowTargetSet);
	safeDestroy(m_lightMapTargetSet);
	safeDestroy(m_normalTargetSet);
	safeDestroy(m_depthTargetSet);

	m_renderView = 0;
}

void WorldRendererPreLit::createRenderView(const WorldViewPerspective& worldView, WorldRenderView& outRenderView) const
{
	float viewNearZ = m_settings.viewNearZ;
	float viewFarZ = m_settings.viewFarZ;
	float depthRange = m_settings.depthRange;

	Frustum viewFrustum;
	viewFrustum.buildPerspective(worldView.fov, worldView.aspect, viewNearZ, viewFarZ);

	outRenderView.setViewSize(Vector2(float(worldView.width), float(worldView.height)));
	outRenderView.setViewFrustum(viewFrustum);
	outRenderView.setCullFrustum(viewFrustum);
	outRenderView.setProjection(perspectiveLh(worldView.fov, worldView.aspect, viewNearZ, viewFarZ));
}

void WorldRendererPreLit::createRenderView(const WorldViewOrtho& worldView, WorldRenderView& outRenderView) const
{
	float viewFarZ = m_settings.viewFarZ;
	float depthRange = m_settings.depthRange;

	Frustum viewFrustum;
	viewFrustum.buildOrtho(worldView.width, worldView.height, -viewFarZ, viewFarZ);

	outRenderView.setViewSize(Vector2(worldView.width, worldView.height));
	outRenderView.setViewFrustum(viewFrustum);
	outRenderView.setCullFrustum(viewFrustum);
	outRenderView.setProjection(orthoLh(worldView.width, worldView.height, -viewFarZ, viewFarZ));
}

void WorldRendererPreLit::build(WorldRenderView& worldRenderView, Entity* entity, int frame)
{
	Frame& f = m_frames[frame];

	// Flush previous frame.
	if (f.haveDepth)
		f.depth->getRenderContext()->flush();

	if (f.haveNormal)
		f.normal->getRenderContext()->flush();

	for (uint32_t i = 0; i < f.lightCount; ++i)
	{
		if (f.haveShadows[i])
		{
			for (int32_t j = 0; j < m_settings.shadowCascadingSlices; ++j)
				f.slice[j].shadow[i]->getRenderContext()->flush();
		}
	}

	f.visual->getRenderContext()->flush();

	// Build depth context.
	{
		WorldRenderView depthRenderView = worldRenderView;
		depthRenderView.resetLights();

		WorldRenderPassPreLit depthPass(
			ms_techniqueDepth,
			false,
			depthRenderView,
			m_settings.depthRange,
			0,
			0,
			0
		);
		f.depth->build(depthRenderView, depthPass, entity);
		f.depth->flush(depthRenderView, depthPass);

		f.haveDepth = true;
	}

	// Build normal context.
	{
		WorldRenderView normalRenderView = worldRenderView;
		normalRenderView.resetLights();

		WorldRenderPassPreLit normalPass(
			ms_techniqueNormal,
			false,
			normalRenderView,
			m_settings.depthRange,
			0,
			0,
			0
		);
		f.normal->build(normalRenderView, normalPass, entity);
		f.normal->flush(normalRenderView, normalPass);

		f.haveNormal = true;
	}

	// Build shadow contexts.
	if (m_settings.shadowsEnabled)
		buildLightWithShadows(worldRenderView, entity, frame);
	else
		buildLightWithNoShadows(worldRenderView, entity, frame);

	// Build visual context.
	buildVisual(worldRenderView, entity, frame);

	m_count++;
}

void WorldRendererPreLit::render(uint32_t flags, int frame, render::EyeType eye)
{
	Frame& f = m_frames[frame];
	Matrix44 projection;

	// Prepare stereoscopic projection.
	if (eye != render::EtCyclop)
	{
		float screenWidth = float(m_renderView->getWidth());
		
		float A = std::abs((c_distortionValue * c_interocularDistance) / screenWidth);
		float B = std::abs(A * c_screenPlaneDistance * (1.0f / f.projection(1, 1)));

		if (eye == render::EtLeft)
			A = -A;
		else
			B = -B;

		projection = translate(A, 0.0f, 0.0f) * f.projection * translate(B, 0.0f, 0.0f);
	}
	else
	{
		projection = f.projection;
	}

	// Prepare global program parameters.
	render::ProgramParameters programParams;
	programParams.beginParameters(m_globalContext);
	programParams.setMatrixParameter(ms_handleProjection, projection);
	programParams.endParameters(m_globalContext);

	// Render depth map; use as z-prepass if able to share depth buffer with primary.
	if ((flags & WrfDepthMap) != 0)
	{
		T_RENDER_PUSH_MARKER(m_renderView, "World: Depth");
		if (m_renderView->begin(m_depthTargetSet, 0))
		{
			if (f.haveDepth)
			{
				const float depthColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
				m_renderView->clear(render::CfColor | render::CfDepth, depthColor, 1.0f, 0);
				f.depth->getRenderContext()->render(m_renderView, render::RfOpaque, &programParams);
			}
			else
			{
				const float nullColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
				m_renderView->clear(render::CfDepth, nullColor, 1.0f, 0);
			}
			m_renderView->end();
		}
		T_RENDER_POP_MARKER(m_renderView);
	}

	// Render normal map.
	if ((flags & WrfDepthMap) != 0)
	{
		T_RENDER_PUSH_MARKER(m_renderView, "World: Normal");
		if (m_renderView->begin(m_normalTargetSet, 0))
		{
			const float depthColor[] = { 0.5f, 0.5f, 0.5f, 0.5f };
			m_renderView->clear(render::CfColor, depthColor, 1.0f, 0);
			if (f.haveNormal)
				f.normal->getRenderContext()->render(m_renderView, render::RfOpaque, &programParams);
			m_renderView->end();
		}
		T_RENDER_POP_MARKER(m_renderView);
	}

	// Render light map.
	if (f.lightCount > 0)
	{
		for (uint32_t i = 0; i < f.lightCount; ++i)
		{
			// Combine all shadow slices into a screen shadow mask.
			if ((flags & WrfShadowMap) != 0 && f.haveShadows[i])
			{
				for (int32_t j = 0; j < m_settings.shadowCascadingSlices; ++j)
				{
					T_RENDER_PUSH_MARKER(m_renderView, "World: Shadow map");
					if (m_renderView->begin(m_shadowTargetSet, 0))
					{
						const float shadowClear[] = { 1.0f, 1.0f, 1.0f, 1.0f };
						m_renderView->clear(render::CfColor | render::CfDepth, shadowClear, 1.0f, 0);
						f.slice[j].shadow[i]->getRenderContext()->render(m_renderView, render::RfOpaque, 0);
						m_renderView->end();
					}
					T_RENDER_POP_MARKER(m_renderView);

					T_RENDER_PUSH_MARKER(m_renderView, "World: Shadow mask project");
					if (m_renderView->begin(m_shadowMaskProjectTargetSet, 0))
					{
						if (j == 0)
						{
							const float maskClear[] = { 1.0f, 1.0f, 1.0f, 1.0f };
							m_renderView->clear(render::CfColor, maskClear, 0.0f, 0);
						}

						Scalar zn(max(m_slicePositions[j], m_settings.viewNearZ));
						Scalar zf(min(m_slicePositions[j + 1], m_settings.shadowFarZ));

						PostProcessStep::Instance::RenderParams params;
						params.viewFrustum = f.viewFrustum;
						params.viewToLight = f.slice[j].viewToLightSpace[i];
						params.projection = projection;
						params.squareProjection = f.slice[j].squareProjection[i];
						params.depthRange = m_settings.depthRange;
						params.sliceNearZ = zn;
						params.sliceFarZ = zf;
						params.shadowMapBias = m_settings.shadowMapBias;
						params.deltaTime = 0.0f;

						m_shadowMaskProject->render(
							m_renderView,
							m_shadowTargetSet,
							m_depthTargetSet,
							0,
							params
						);

						m_renderView->end();
					}
					T_RENDER_POP_MARKER(m_renderView);
				}

				T_RENDER_PUSH_MARKER(m_renderView, "World: Shadow mask filter");
				if (m_renderView->begin(m_shadowMaskFilterTargetSet, 0))
				{
					PostProcessStep::Instance::RenderParams params;
					params.viewFrustum = f.viewFrustum;
					params.projection = projection;
					params.depthRange = m_settings.depthRange;
					params.sliceNearZ = 0.0f;
					params.sliceFarZ = m_settings.shadowFarZ;
					params.shadowMapBias = m_settings.shadowMapBias;
					params.deltaTime = 0.0f;

					m_shadowMaskFilter->render(
						m_renderView,
						m_shadowMaskProjectTargetSet,
						m_depthTargetSet,
						0,
						params
					);
					m_renderView->end();
				}
				T_RENDER_POP_MARKER(m_renderView);
			}

			T_RENDER_PUSH_MARKER(m_renderView, "World: Light primitive");
			if (m_renderView->begin(m_lightMapTargetSet, 0))
			{
				if (i == 0)
				{
					const float lightClear[] = { 0.0f, 0.0f, 0.0f, 0.0f };
					m_renderView->clear(render::CfColor, lightClear, 0.0f, 0);
				}
				m_lightRenderer->render(
					m_renderView,
					f.projection,
					f.view,
					f.eyePosition,
					f.lights[i],
					m_settings.depthRange,
					m_depthTargetSet->getColorTexture(0),
					m_normalTargetSet->getColorTexture(0),
					f.haveShadows[i] ? m_shadowMaskFilterTargetSet->getWidth() : 0,
					f.haveShadows[i] ? m_shadowMaskFilterTargetSet->getColorTexture(0) : 0
				);
				m_renderView->end();
			}
			T_RENDER_POP_MARKER(m_renderView);
		}
	}
	else
	{
		// No active lights; ensure light map is cleared.
		T_RENDER_PUSH_MARKER(m_renderView, "World: Light map");
		if (m_renderView->begin(m_lightMapTargetSet, 0))
		{
			const float lightClear[] = { 0.0f, 0.0f, 0.0f, 0.0f };
			m_renderView->clear(render::CfColor, lightClear, 0.0f, 0);
			m_renderView->end();
		}
		T_RENDER_POP_MARKER(m_renderView);
	}

	// Render visuals.
	if ((flags & (WrfVisualOpaque | WrfVisualAlphaBlend)) != 0)
	{
		uint32_t renderFlags = render::RfOverlay;

		if (flags & WrfVisualOpaque)
			renderFlags |= render::RfOpaque;
		if (flags & WrfVisualAlphaBlend)
			renderFlags |= render::RfAlphaBlend;

		T_RENDER_PUSH_MARKER(m_renderView, "World: Visual");
		f.visual->getRenderContext()->render(m_renderView, renderFlags, &programParams);
		T_RENDER_POP_MARKER(m_renderView);
	}

	m_globalContext->flush();
}

render::RenderTargetSet* WorldRendererPreLit::getDepthTargetSet()
{
	return m_depthTargetSet;
}

render::RenderTargetSet* WorldRendererPreLit::getShadowMaskTargetSet()
{
	return m_shadowMaskFilterTargetSet;
}

void WorldRendererPreLit::getTargets(RefArray< render::ITexture >& outTargets) const
{
	outTargets.resize(4);
	outTargets[0] = m_depthTargetSet ? m_depthTargetSet->getColorTexture(0) : 0;
	outTargets[1] = m_normalTargetSet ? m_normalTargetSet->getColorTexture(0) : 0;
	outTargets[2] = m_lightMapTargetSet ? m_lightMapTargetSet->getColorTexture(0) : 0;
	outTargets[3] = m_shadowMaskFilterTargetSet ? m_shadowMaskFilterTargetSet->getColorTexture(0) : 0;
}

void WorldRendererPreLit::buildLightWithShadows(WorldRenderView& worldRenderView, Entity* entity, int frame)
{
	Frame& f = m_frames[frame];

	Matrix44 viewInverse = worldRenderView.getView().inverse();
	Frustum viewFrustum = worldRenderView.getViewFrustum();
	Aabb3 shadowBox = worldRenderView.getShadowBox();

	f.eyePosition = worldRenderView.getEyePosition();
	f.lightCount = worldRenderView.getLightCount();

	for (int32_t i = 0; i < worldRenderView.getLightCount(); ++i)
	{
		const Light& light = worldRenderView.getLight(i);

		f.lights[i] = light;

		if (light.type == LtDirectional && light.castShadow)
		{
			for (int32_t slice = 0; slice < m_settings.shadowCascadingSlices; ++slice)
			{
				Scalar zn(max(m_slicePositions[slice], m_settings.viewNearZ));
				Scalar zf(min(m_slicePositions[slice + 1], m_settings.shadowFarZ));

				// Create sliced view frustum.
				Frustum sliceViewFrustum = viewFrustum;
				sliceViewFrustum.setNearZ(zn);
				sliceViewFrustum.setFarZ(zf);

				// Calculate shadow map projection.
				Matrix44 shadowLightView;
				Matrix44 shadowLightProjection;
				Matrix44 shadowLightSquareProjection = Matrix44::identity();
				Frustum shadowFrustum;

				switch (m_settings.shadowsProjection)
				{
				case WorldRenderSettings::SpBox:
					calculateBoxSMProj(
						m_settings,
						viewInverse,
						light.position,
						light.direction,
						sliceViewFrustum,
						shadowBox,
						shadowLightView,
						shadowLightProjection,
						shadowLightSquareProjection,
						shadowFrustum
					);
					break;

				case WorldRenderSettings::SpLiSP:
					calculateLiSPSMProj(
						m_settings,
						viewInverse,
						light.position,
						light.direction,
						sliceViewFrustum,
						shadowLightView,
						shadowLightProjection,
						shadowLightSquareProjection,
						shadowFrustum
					);
					break;

				case WorldRenderSettings::SpTrapezoid:
					calculateTSMProj(
						m_settings,
						viewInverse,
						light.position,
						light.direction,
						sliceViewFrustum,
						shadowLightView,
						shadowLightProjection,
						shadowLightSquareProjection,
						shadowFrustum
					);
					break;

				case WorldRenderSettings::SpUniform:
					calculateUniformSMProj(
						m_settings,
						viewInverse,
						light.position,
						light.direction,
						sliceViewFrustum,
						shadowLightView,
						shadowLightProjection,
						shadowLightSquareProjection,
						shadowFrustum
					);
					break;
				}

				// Render shadow map.
				WorldRenderView shadowRenderView;
				shadowRenderView.resetLights();
				shadowRenderView.setProjection(shadowLightProjection);
				shadowRenderView.setSquareProjection(shadowLightSquareProjection);
				shadowRenderView.setView(shadowLightView);
				shadowRenderView.setEyePosition(f.eyePosition);
				shadowRenderView.setViewFrustum(shadowFrustum);
				shadowRenderView.setCullFrustum(shadowFrustum);
				shadowRenderView.setTimes(
					worldRenderView.getTime(),
					worldRenderView.getDeltaTime(),
					worldRenderView.getInterval()
				);

				WorldRenderPassPreLit shadowPass(
					ms_techniqueShadow,
					false,
					shadowRenderView,
					m_settings.depthRange,
					0,
					0,
					0
				);
				f.slice[slice].shadow[i]->build(shadowRenderView, shadowPass, entity);
				f.slice[slice].shadow[i]->flush(shadowRenderView, shadowPass);
				
				f.slice[slice].viewToLightSpace[i] = shadowLightProjection * shadowLightView * viewInverse;
				f.slice[slice].squareProjection[i] = shadowLightSquareProjection;
			}

			f.haveShadows[i] = true;
		}
		else
			f.haveShadows[i] = false;
	}
}

void WorldRendererPreLit::buildLightWithNoShadows(WorldRenderView& worldRenderView, Entity* entity, int frame)
{
	Frame& f = m_frames[frame];
	f.eyePosition = worldRenderView.getEyePosition();
	f.lightCount = worldRenderView.getLightCount();
	for (int32_t i = 0; i < worldRenderView.getLightCount(); ++i)
	{
		const Light& light = worldRenderView.getLight(i);
		f.lights[i] = light;
	}
}

void WorldRendererPreLit::buildVisual(WorldRenderView& worldRenderView, Entity* entity, int frame)
{
	Frame& f = m_frames[frame];

	Matrix44 viewInverse = worldRenderView.getView().inverse();
	Frustum viewFrustum = worldRenderView.getViewFrustum();
	Aabb3 shadowBox = worldRenderView.getShadowBox();
	Vector4 eyePosition = viewInverse.translation();

	worldRenderView.resetLights();
	worldRenderView.setEyePosition(eyePosition);

	WorldRenderPassPreLit defaultPreLitPass(
		ms_techniquePreLitColor,
		true,
		worldRenderView,
		m_settings.depthRange,
		f.haveDepth ? m_depthTargetSet->getColorTexture(0) : 0,
		0,
		m_lightMapTargetSet->getColorTexture(0)
	);
	f.visual->build(worldRenderView, defaultPreLitPass, entity);
	f.visual->flush(worldRenderView, defaultPreLitPass);

	f.projection = worldRenderView.getProjection();
	f.view = worldRenderView.getView();
	f.viewFrustum = worldRenderView.getViewFrustum();
}

	}
}
