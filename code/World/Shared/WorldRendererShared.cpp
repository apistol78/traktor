/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Shared/WorldRendererShared.h"

#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Timer/Profiler.h"
#include "Render/Buffer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Resource/IResourceManager.h"
#include "World/Entity.h"
#include "World/Entity/ColorCorrectionComponent.h"
#include "World/Entity/FogComponent.h"
#include "World/Entity/IrradianceGridComponent.h"
#include "World/Entity/LightComponent.h"
#include "World/Entity/PostProcessComponent.h"
#include "World/Entity/ProbeComponent.h"
#include "World/Entity/RTWorldComponent.h"
#include "World/IEntityRenderer.h"
#include "World/IrradianceGrid.h"
#include "World/IWorldComponent.h"
#include "World/Packer.h"
#include "World/Shared/Passes/AmbientOcclusionPass.h"
#include "World/Shared/Passes/ContactShadowsPass.h"
#include "World/Shared/Passes/DBufferPass.h"
#include "World/Shared/Passes/DownScalePass.h"
#include "World/Shared/Passes/EntityIdPass.h"
#include "World/Shared/Passes/GBufferPass.h"
#include "World/Shared/Passes/HiZPass.h"
#include "World/Shared/Passes/IrradiancePass.h"
#include "World/Shared/Passes/LightClusterPass.h"
#include "World/Shared/Passes/PostDepthPass.h"
#include "World/Shared/Passes/PostProcessPass.h"
#include "World/Shared/Passes/ReflectionsPass.h"
#include "World/Shared/Passes/VelocityPass.h"
#include "World/Shared/Passes/VolumetricFogPass.h"
#include "World/Shared/Passes/ZPrePass.h"
#include "World/Shared/WorldRenderPassShared.h"
#include "World/SMProj/UniformShadowProjection.h"
#include "World/World.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"

#include <algorithm>
#include <cstring>

namespace traktor::world
{
namespace
{

// Margin (world units) by which a cached shadow slice is expanded before it is rendered,
// so the camera can move a little before the cached slice no longer covers the view and
// must be re-rendered. Scaled with the slice's far distance - far cascades are low
// resolution, so a larger margin is invisible there but lets more frames be skipped -
// with a floor so near slices keep a small but non-zero margin.
const Scalar c_shadowSliceMarginFactor(0.02f);
const Scalar c_shadowSliceMarginMin(1.0f);

// Factor by which the furthest measured depth is widened before it is used to cull
// slice updates. The measurement is a few frames old, so geometry may have come into
// view since it was taken; the margin is what covers that.
const float c_sliceFarMargin = 1.25f;

// Maximum number of shadow casting point lights; each appends six cube face entries to the light buffer.
const int32_t c_maxShadowPointLights = 16;

// Border, in texels, around each cube face so the shadow filter kernel stays inside the face.
const int32_t c_pointShadowFaceBorder = 2;

const float c_pointShadowNearZ = 0.1f;

// View to light translation of an entry without shadow; projects outside of atlas tile so light is unshadowed.
const Vector4 c_unshadowedTranslation(2.0f, 2.0f, 0.0f, 1.0f);

// Longest indirect ray, lights further than this from any lit point cannot contribute.
const Scalar c_indirectReach(20.0f);

// Half extent of camera centered irradiance field volume.
const Scalar c_irradianceFieldExtent(40.0f);

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
	const WorldCreateDesc& desc)
{
	m_entityRenderers = desc.entityRenderers;

	// Store settings.
	m_settings = *desc.worldRenderSettings;
	m_shadowsQuality = desc.quality.shadows;
	m_rayTracingEnabled = desc.rt;

	// Create screen renderer.
	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	// Create default value textures.
	m_blackTexture = create1x1Texture(renderSystem, 0x00000000);
	if (!m_blackTexture)
		return false;
	m_whiteTexture = create1x1Texture(renderSystem, 0xffffffff);
	if (!m_whiteTexture)
		return false;
	m_blackCubeTexture = createCubeTexture(renderSystem, 0x00000000);
	if (!m_blackCubeTexture)
		return false;

	// Lights struct buffer.
	for (int32_t i = 0; i < sizeof_array(m_state); ++i)
	{
		m_state[i].lightSBuffer = renderSystem->createBuffer(
			render::BuStructured,
			(LightClusterPass::c_maxLightCount + 3 + c_maxShadowPointLights * 6) * sizeof(LightShaderData),
			true,
			T_FILE_LINE_W);
		if (!m_state[i].lightSBuffer)
			return false;
	}

	const auto& shadowSettings = m_settings.shadowSettings[(int32_t)m_shadowsQuality];
	m_shadowAtlasPacker = new Packer(
		shadowSettings.resolution,
		shadowSettings.resolution);

	// Determine slice distances; these always stay as configured, only how far the
	// cascades need to reach is adjusted by the measured depth range each frame.
	for (int32_t i = 0; i < sizeof_array(m_state); ++i)
	{
		for (int32_t j = 0; j <= shadowSettings.cascadingSlices; ++j)
		{
			const float jj = float(j) / shadowSettings.cascadingSlices;
			const float log = powf(jj, shadowSettings.cascadingLambda);
			m_state[i].slicePositions[j] = lerp(m_settings.viewNearZ, shadowSettings.farZ, log);
		}
		for (int32_t j = shadowSettings.cascadingSlices + 1; j <= MaxSliceCount; ++j)
			m_state[i].slicePositions[j] = shadowSettings.farZ;

		m_state[i].sliceCullFarZ = shadowSettings.farZ;
	}

	// Create shared passes.
	m_lightClusterPass = new LightClusterPass(m_settings);
	if (!m_lightClusterPass->create(renderSystem))
		return false;

	m_zPrePass = new ZPrePass(m_settings, m_entityRenderers);
	m_gbufferPass = new GBufferPass(m_settings, m_entityRenderers);
	m_dbufferPass = new DBufferPass(m_settings);

	m_downScalePass = new DownScalePass();
	if (!m_downScalePass->create(resourceManager, renderSystem))
		return false;

	m_hiZPass = new HiZPass();
	if (!m_hiZPass->create(resourceManager))
		return false;

	m_velocityPass = new VelocityPass();
	if (!m_velocityPass->create(resourceManager, renderSystem, desc))
		return false;

	m_irradiancePass = new IrradiancePass();
	if (!m_irradiancePass->create(resourceManager, renderSystem, desc))
		return false;

	m_ambientOcclusionPass = new AmbientOcclusionPass();
	if (!m_ambientOcclusionPass->create(resourceManager, renderSystem, desc))
		return false;

	m_volumetricFogPass = new VolumetricFogPass(m_settings);
	if (!m_volumetricFogPass->create(resourceManager, renderSystem, desc))
		return false;

	m_contactShadowsPass = new ContactShadowsPass();
	if (!m_contactShadowsPass->create(resourceManager, renderSystem, desc))
		return false;

	m_reflectionsPass = new ReflectionsPass();
	if (!m_reflectionsPass->create(resourceManager, renderSystem, desc))
		return false;

	m_postDepthPass = new PostDepthPass();
	if (!m_postDepthPass->create(resourceManager, renderSystem, desc))
		return false;

	m_postProcessPass = new PostProcessPass(m_settings);
	if (!m_postProcessPass->create(resourceManager, renderSystem, desc))
		return false;

	// Entity id pass is optional; only created when queried, which only the editor does.
	if (desc.entityIdQuery)
	{
		m_entityIdPass = new EntityIdPass();
		if (!m_entityIdPass->create(resourceManager, renderSystem, desc))
			m_entityIdPass = nullptr;
	}

	return true;
}

void WorldRendererShared::destroy()
{
	safeDestroy(m_screenRenderer);
	safeDestroy(m_blackCubeTexture);
	safeDestroy(m_blackTexture);
	safeDestroy(m_whiteTexture);

	for (int32_t i = 0; i < sizeof_array(m_state); ++i)
		safeDestroy(m_state[i].lightSBuffer);

	safeDestroy(m_entityIdPass);
	safeDestroy(m_postProcessPass);
	safeDestroy(m_postDepthPass);
	safeDestroy(m_reflectionsPass);
	safeDestroy(m_contactShadowsPass);
	safeDestroy(m_volumetricFogPass);
	safeDestroy(m_ambientOcclusionPass);
	safeDestroy(m_irradiancePass);
	safeDestroy(m_velocityPass);
	safeDestroy(m_hiZPass);
	safeDestroy(m_downScalePass);
	safeDestroy(m_dbufferPass);
	safeDestroy(m_gbufferPass);
	safeDestroy(m_zPrePass);
	safeDestroy(m_lightClusterPass);

	m_entityRenderers = nullptr;
	m_gatheredView = {};
}

void WorldRendererShared::gather(const World* world, const WorldRenderView& worldRenderView, const std::function< bool(const EntityState& state) >& filter)
{
	T_PROFILER_SCOPE(L"WorldRendererShared::gather");
	StaticVector< const LightComponent*, LightClusterPass::c_maxLightCount > lights;

	const Matrix44& view = worldRenderView.getView();
	const Frustum& viewFrustum = worldRenderView.getViewFrustum();
	const Vector4 eyePosition = worldRenderView.getEyePosition();

	m_gatheredView.renderables.reset();
	m_gatheredView.lights.resize(0);
	m_gatheredView.probes.resize(0);
	m_gatheredView.colorCorrection = nullptr;
	m_gatheredView.fog = nullptr;
	m_gatheredView.postProcess = nullptr;
	m_gatheredView.irradianceGrid = nullptr;
	m_gatheredView.rtWorldTopLevel = nullptr;

	for (auto entity : world->getEntities())
	{
		const EntityState state = entity->getState();

		if (filter != nullptr && filter(state) == false)
			continue;
		else if (filter == nullptr && state.visible == false)
			continue;

		for (auto component : entity->getComponents())
		{
			IEntityRenderer* entityRenderer = m_entityRenderers->find(type_of(component));
			if (entityRenderer)
			{
				auto& r = m_gatheredView.renderables[entityRenderer];
				r.objects.push_back(component);
				if (!state.dynamic)
					r.staticOnlyObjects.push_back(component);
			}

			// Filter out components used to setup frame's lighting etc.
			if (auto lightComponent = dynamic_type_cast< const LightComponent* >(component))
			{
				if (lightComponent->getLightType() == LightType::Disabled || lights.full())
					continue;

				// Skip local light if its range, widened by indirect reach, touches neither view frustum nor irradiance field.
				if (lightComponent->getLightType() == LightType::Point || lightComponent->getLightType() == LightType::Spot)
				{
					const Vector4 position = lightComponent->getTransform().translation().xyz1();
					const Scalar radius = lightComponent->getFarRange() + c_indirectReach;
					if (
						viewFrustum.inside(view * position, radius) == Frustum::Result::Outside &&
						(position - eyePosition).xyz0().absolute().max() > c_irradianceFieldExtent + radius
					)
						continue;
				}

				lights.push_back(lightComponent);
			}
			else if (auto probeComponent = dynamic_type_cast< const ProbeComponent* >(component))
				m_gatheredView.probes.push_back(probeComponent);
		}
	}

	for (auto component : world->getComponents())
	{
		IEntityRenderer* entityRenderer = m_entityRenderers->find(type_of(component));
		if (entityRenderer)
		{
			auto& r = m_gatheredView.renderables[entityRenderer];
			r.objects.push_back(component);
			r.staticOnlyObjects.push_back(component);
		}

		// Filter out components used to setup frame's lighting etc.
		if (auto irradianceGridComponent = dynamic_type_cast< const IrradianceGridComponent* >(component))
			m_gatheredView.irradianceGrid = irradianceGridComponent->getIrradianceGrid();
		else if (auto fogComponent = dynamic_type_cast< const FogComponent* >(component))
			m_gatheredView.fog = fogComponent;
		else if (auto colorCorrectionComponent = dynamic_type_cast< const ColorCorrectionComponent* >(component))
			m_gatheredView.colorCorrection = colorCorrectionComponent;
		else if (auto postProcessComponent = dynamic_type_cast< const PostProcessComponent* >(component))
			m_gatheredView.postProcess = postProcessComponent;
		else if (m_rayTracingEnabled)
		{
			if (auto rtWorldComponent = dynamic_type_cast< const RTWorldComponent* >(component))
				m_gatheredView.rtWorldTopLevel = rtWorldComponent->getTopLevel();
		}
	}

	// Arrange lights.
	{
		m_gatheredView.lights.resize(0);
		m_gatheredView.cascadingDirectionalLight = nullptr;

		// Find cascade shadow directional light.
		const bool shadowsEnable = (bool)(m_shadowsQuality != Quality::Disabled);
		if (shadowsEnable)
		{
			for (int32_t i = 0; i < (int32_t)lights.size(); ++i)
			{
				auto& light = lights[i];
				if (
					light->getCastShadow() &&
					light->getLightType() == LightType::Directional)
				{
					m_gatheredView.cascadingDirectionalLight = light;
					break;
				}
			}
		}

		// Add all lights, skip cascade shadow directional light.
		for (int32_t i = 0; i < (int32_t)lights.size(); ++i)
		{
			auto& light = lights[i];
			if (light != m_gatheredView.cascadingDirectionalLight)
				m_gatheredView.lights.push_back(light);
		}

		// Append cascade shadow directional light last.
		if (m_gatheredView.cascadingDirectionalLight != nullptr)
			m_gatheredView.lights.push_back(m_gatheredView.cascadingDirectionalLight);
	}
}

void WorldRendererShared::setupSliceCullDistance(
	const WorldRenderView& worldRenderView,
	State& state)
{
	T_PROFILER_SCOPE(L"WorldRendererShared setupSliceCullDistance");

	if (m_shadowsQuality == Quality::Disabled)
		return;

	const auto& shadowSettings = m_settings.shadowSettings[(int32_t)m_shadowsQuality];

	// The cascade splits always stay as configured so the resolution distribution is
	// stable while moving and looking around; the measured depth range is only used
	// to cull updates of slices which are entirely beyond the furthest visible depth,
	// and to clamp the far distance of the last visible slice.
	float measuredNearZ = 0.0f;
	float measuredFarZ = 0.0f;
	if (!m_downScalePass->getDepthRange(worldRenderView, measuredNearZ, measuredFarZ))
	{
		// Without a measurement - which is the case for the first few frames, and
		// whenever nothing but background is in view - the cascades have to cover
		// everything. This also guarantees every slice is rendered at least once
		// before culling can begin, so no slice exposes uninitialized state.
		state.sliceCullFarZ = shadowSettings.farZ;
		return;
	}

	// Widen beyond the measured depth; the measurement is a few frames old, so
	// geometry may have come into view since it was taken.
	const float cullFarZ = std::min(measuredFarZ * c_sliceFarMargin, shadowSettings.farZ);
	state.sliceCullFarZ = std::max(cullFarZ, m_settings.viewNearZ);
}

render::RGTargetSet WorldRendererShared::setupLightPass(
	const WorldRenderView& worldRenderView,
	render::RenderGraph& renderGraph)
{
	T_PROFILER_SCOPE(L"WorldRendererShared setupLightPass");

	const auto& shadowSettings = m_settings.shadowSettings[(int32_t)m_shadowsQuality];
	const bool shadowMapDirectionalEnable = (bool)(m_shadowsQuality != Quality::Disabled);
	const bool shadowMapAtlasEnable = (bool)(m_shadowsQuality != Quality::Disabled && m_gatheredView.rtWorldTopLevel == nullptr);
	const UniformShadowProjection shadowProjection(shadowSettings.resolution);

	T_FATAL_ASSERT(worldRenderView.getIndex() < sizeof_array(m_state));
	State& state = m_state[worldRenderView.getIndex()];

	// Determine how far the cascades need to reach from the measured depth range.
	setupSliceCullDistance(worldRenderView, state);

	LightShaderData* lightShaderData = (LightShaderData*)m_state[worldRenderView.getIndex()].lightSBuffer->lock();
	if (!lightShaderData)
		return render::RGTargetSet::Invalid;

	render::RGTargetSet shadowMapAtlasTargetSetId;

	// Reset this frame's atlas packer.
	auto shadowAtlasPacker = m_shadowAtlasPacker;
	shadowAtlasPacker->reset();

	const Matrix44 view = worldRenderView.getView();
	const Matrix44 viewInverse = worldRenderView.getView().inverse();
	const Frustum viewFrustum = worldRenderView.getViewFrustum();

	Frustum* shadowSlices = state.shadowSlices;
	Matrix44* sliceViews = state.sliceViews;
	Matrix44* shadowLightViews = state.shadowLightViews;

	// Find atlas shadow lights.
	StaticVector< int32_t, 32 > lightAtlasIndices;
	AlignedVector< int32_t > lightPointIndices;
	if (shadowMapAtlasEnable)
	{
		for (int32_t i = 0; i < (int32_t)m_gatheredView.lights.size(); ++i)
		{
			const auto& light = m_gatheredView.lights[i];
			if (!light->getCastShadow())
				continue;
			if (light->getLightType() == LightType::Spot && !lightAtlasIndices.full())
				lightAtlasIndices.push_back(i);
			else if (light->getLightType() == LightType::Point)
				lightPointIndices.push_back(i);
		}

		// Nearest point lights first so they get largest atlas regions.
		const Vector4 eyePosition = worldRenderView.getEyePosition();
		std::sort(lightPointIndices.begin(), lightPointIndices.end(), [&](int32_t lh, int32_t rh) {
			const Scalar dl = (m_gatheredView.lights[lh]->getTransform().translation() - eyePosition).xyz0().length2();
			const Scalar dr = (m_gatheredView.lights[rh]->getTransform().translation() - eyePosition).xyz0().length2();
			return dl < dr;
		});
		if (lightPointIndices.size() > c_maxShadowPointLights)
			lightPointIndices.resize(c_maxShadowPointLights);
	}

	// Write all lights to sbuffer; without shadow map information.
	for (int32_t i = 0; i < (int32_t)m_gatheredView.lights.size(); ++i)
	{
		const auto& light = m_gatheredView.lights[i];
		auto* lsd = &lightShaderData[i];

		lsd->type = (float)light->getLightType();
		lsd->shadowIndex = 0.0f;
		lsd->rangeRadius[0] = light->getNearRange();
		lsd->rangeRadius[1] = light->getFarRange();
		// Cosine of the half angle of the inner, fully lit, and the outer cone of a spot light.
		lsd->rangeRadius[2] = std::cos((light->getRadius() - deg2rad(c_spotLightPenumbraAngle)) / 2.0f);
		lsd->rangeRadius[3] = std::cos(light->getRadius() / 2.0f);

		const Matrix44 lightTransform = view * light->getTransform().toMatrix44();
		lightTransform.translation().xyz1().storeUnaligned(lsd->position);
		lightTransform.axisY().xyz0().storeUnaligned(lsd->direction);
		(light->getColor() * light->getFlickerCoeff()).storeUnaligned(lsd->color);

		Vector4::zero().storeUnaligned(lsd->viewToLight0);
		Vector4::zero().storeUnaligned(lsd->viewToLight1);
		Vector4::zero().storeUnaligned(lsd->viewToLight2);
		c_unshadowedTranslation.storeUnaligned(lsd->viewToLight3);
		Vector4::zero().storeUnaligned(lsd->atlasTransform);
	}

	for (int32_t i = (int32_t)m_gatheredView.lights.size(); i < (int32_t)m_gatheredView.lights.size() + 3; ++i)
	{
		auto* lsd = &lightShaderData[i];

		lsd->type = 0.0f;
		lsd->shadowIndex = 0.0f;

		Vector4::zero().storeUnaligned(lsd->viewToLight0);
		Vector4::zero().storeUnaligned(lsd->viewToLight1);
		Vector4::zero().storeUnaligned(lsd->viewToLight2);
		c_unshadowedTranslation.storeUnaligned(lsd->viewToLight3);
		Vector4::zero().storeUnaligned(lsd->atlasTransform);
	}

	// If shadow casting directional light found add cascade shadow map pass
	// and update light sbuffer.
	if (shadowMapDirectionalEnable)
	{
		const int32_t cascadingSlices = (m_gatheredView.cascadingDirectionalLight != nullptr) ? shadowSettings.cascadingSlices : 0;
		const int32_t shmw = shadowSettings.resolution * (cascadingSlices + 1);
		const int32_t shmh = shadowSettings.resolution;
		const int32_t sliceDim = shadowSettings.resolution;
		const int32_t atlasOffset = shadowSettings.resolution * cascadingSlices;
		int32_t shadowMapIndex = 0;

		// Add shadow map target.
		render::RenderGraphTargetSetDesc rgtd;
		rgtd.count = 0;
		rgtd.width = shmw;
		rgtd.height = shmh;
		rgtd.createDepthStencil = true;
		rgtd.usingDepthStencilAsTexture = true;
		rgtd.ignoreStencil = true;
		shadowMapAtlasTargetSetId = renderGraph.addPersistentTargetSet(
			L"Shadow map atlas",
			ShaderParameter::TargetShadowMap[worldRenderView.getIndex()],
			false,
			rgtd);

		// Add shadow map render passes.
		Ref< render::RenderPass > rp = new render::RenderPass(L"Shadow map");
		for (const auto& attachment : m_gatheredView.setupAttachments)
			rp->addInput(attachment);
		rp->setOutput(shadowMapAtlasTargetSetId, render::TfDepth, render::TfDepth);

		// Add render of a single shadow map tile in the atlas.
		const auto addShadowTile = [&](const Packer::Rectangle& tile, const Matrix44& shadowLightProjection, const Matrix44& shadowLightView, const Frustum& shadowFrustum) {
			const int32_t passShadowMapIndex = shadowMapIndex++;

			rp->addBuild(
				[=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
				const WorldBuildContext wc(
					m_entityRenderers,
					renderContext);

				// Render shadow map.
				WorldRenderView shadowRenderView;
				shadowRenderView.setIndex(worldRenderView.getIndex());
				shadowRenderView.setShadowMapIndex(passShadowMapIndex);
				shadowRenderView.setProjection(shadowLightProjection);
				shadowRenderView.setView(shadowLightView, shadowLightView);
				shadowRenderView.setViewFrustum(shadowFrustum);
				shadowRenderView.setCullFrustum(shadowFrustum);
				shadowRenderView.setTimes(
					worldRenderView.getTime(),
					worldRenderView.getDeltaTime(),
					worldRenderView.getInterval());

				// Set viewport to light atlas slot.
				auto svrb = renderContext->alloc< render::SetViewportRenderBlock >();
				svrb->viewport = render::Viewport(
					tile.x,
					tile.y,
					tile.width,
					tile.height,
					0.0f,
					1.0f);
				renderContext->draw(svrb);

				// Render entities into shadow map.
				auto sharedParams = renderContext->alloc< render::ProgramParameters >();
				sharedParams->beginParameters(renderContext);
				sharedParams->setFloatParameter(ShaderParameter::Time, (float)worldRenderView.getTime());
				sharedParams->setMatrixParameter(ShaderParameter::Projection, shadowLightProjection);
				sharedParams->setMatrixParameter(ShaderParameter::View, shadowLightView);
				sharedParams->setMatrixParameter(ShaderParameter::ViewInverse, shadowLightView.inverse());
				sharedParams->endParameters(renderContext);

				const WorldRenderPassShared shadowPass(
					ShaderTechnique::Shadow,
					sharedParams,
					shadowRenderView);

				T_ASSERT(!renderContext->havePendingDraws());

				// Clear shadow map tile; use a region clear so the HW "fast clear"
				// path is utilized instead of a fill primitive.
				{
					auto crb = renderContext->allocNamed< render::ClearRenderBlock >(L"Clear shadow map tile");
					crb->clear.mask = render::CfDepth;
					crb->clear.depth = 1.0f;
					crb->rect = render::Rectangle(tile.x, tile.y, tile.width, tile.height);
					renderContext->draw(crb);
				}

				for (auto it : m_gatheredView.renderables)
				{
					IEntityRenderer* entityRenderer = it.first;
					const GatherView::Renderable& r = it.second;
					entityRenderer->build(wc, shadowRenderView, shadowPass, r.objects);
				}
			});
		};

		if (m_gatheredView.cascadingDirectionalLight != nullptr)
		{
			const LightComponent* light = m_gatheredView.cascadingDirectionalLight;
			const Transform lightTransform = light->getTransform();
			const Vector4 lightPosition = lightTransform.translation().xyz1();
			const Vector4 lightDirection = lightTransform.axisY().xyz0();

			// Update one slice per frame.
			for (int32_t i = 0; i < shadowSettings.cascadingSlices; ++i)
			{
				const int32_t slice = i;
				const Scalar zn(max(state.slicePositions[slice], m_settings.viewNearZ));
				Scalar zf(min(state.slicePositions[slice + 1], shadowSettings.farZ));
				const bool staticOnly = bool(slice >= shadowSettings.cascadingSlices - 2);

				// The first slice contains dynamic entities and is always rendered as
				// configured. Further slices are culled against the furthest measured
				// depth - a slice entirely beyond it has no receivers - and the far
				// distance of the last visible slice is clamped to it so its projection
				// fits the geometry actually in view.
				if (slice != 0)
				{
					const Scalar cullFarZ(state.sliceCullFarZ);
					if (zn >= cullFarZ)
						continue;
					zf = min(zf, cullFarZ);
				}

				// Create sliced view frustum.
				Frustum sliceViewFrustum = viewFrustum;
				sliceViewFrustum.setNearZ(zn);
				sliceViewFrustum.setFarZ(zf);

#if 1
				// Check if this slice is still inside the expanded slice frustum that
				// was rendered into the atlas. shadowSlices[i] is expressed in the view
				// space of the frame it was last rendered (sliceViews[i]); transform the
				// current slice frustum from the current view space into that space so
				// the containment test measures the full accumulated camera motion, not
				// just the delta since the previous frame.
				if (slice != 0)
				{
					const Matrix44 toStored = sliceViews[i] * viewInverse;
					if (!staticOnly)
					{
						// Force one dynamic slice to refresh each frame (round-robin) so
						// moving entities are picked up even when the camera is still. The
						// last slice is static-only (handled below), so rotate only over
						// the dynamic slices 1 .. cascadingSlices-2.
						const int32_t dynamicSlices = std::max< int32_t >(shadowSettings.cascadingSlices - 2, 1);
						const int32_t force = (state.count % dynamicSlices) + 1;
						if (slice != force && shadowSlices[i].inside(toStored, sliceViewFrustum) == Frustum::Result::Inside)
							continue;
					}
					else
					{
						// Slices with no dynamic entities should be safe to always ignore if camera is stationary.
						if (shadowSlices[i].inside(toStored, sliceViewFrustum) == Frustum::Result::Inside)
							continue;
					}
					sliceViewFrustum.scale(max(c_shadowSliceMarginMin, zf * c_shadowSliceMarginFactor));
				}
#endif

				shadowSlices[i] = sliceViewFrustum;
				sliceViews[i] = view;

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
					shadowFrustum);

				shadowLightViews[slice] = shadowLightProjection * shadowLightView;

				const int32_t passShadowMapIndex = shadowMapIndex++;

				rp->addBuild(
					[=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
					WorldBuildContext wc(
						m_entityRenderers,
						renderContext);

					// Render shadow map.
					WorldRenderView shadowRenderView;
					shadowRenderView.setIndex(worldRenderView.getIndex());
					shadowRenderView.setShadowMapIndex(passShadowMapIndex);
					shadowRenderView.setStaticOnly(staticOnly);
					shadowRenderView.setProjection(shadowLightProjection);
					shadowRenderView.setView(shadowLightView, shadowLightView);
					shadowRenderView.setViewFrustum(shadowFrustum);
					shadowRenderView.setCullFrustum(shadowFrustum);
					shadowRenderView.setTimes(
						worldRenderView.getTime(),
						worldRenderView.getDeltaTime(),
						worldRenderView.getInterval());

					// Set viewport to current cascade.
					auto svrb = renderContext->alloc< render::SetViewportRenderBlock >();
					svrb->viewport = render::Viewport(
						slice * sliceDim,
						0,
						sliceDim,
						sliceDim,
						0.0f,
						1.0f);
					renderContext->draw(svrb);

					// Render entities into shadow map.
					auto sharedParams = renderContext->alloc< render::ProgramParameters >();
					sharedParams->beginParameters(renderContext);
					sharedParams->setFloatParameter(ShaderParameter::Time, (float)worldRenderView.getTime());
					sharedParams->setMatrixParameter(ShaderParameter::Projection, shadowLightProjection);
					sharedParams->setMatrixParameter(ShaderParameter::View, shadowLightView);
					sharedParams->setMatrixParameter(ShaderParameter::ViewInverse, shadowLightView.inverse());
					sharedParams->endParameters(renderContext);

					const WorldRenderPassShared shadowPass(
						ShaderTechnique::Shadow,
						sharedParams,
						shadowRenderView);

					T_ASSERT(!renderContext->havePendingDraws());

					// Clear cascade shadow map slice; use a region clear so the HW
					// "fast clear" path is utilized instead of a fill primitive.
					{
						auto crb = renderContext->allocNamed< render::ClearRenderBlock >(L"Clear shadow map slice");
						crb->clear.mask = render::CfDepth;
						crb->clear.depth = 1.0f;
						crb->rect = render::Rectangle(slice * sliceDim, 0, sliceDim, sliceDim);
						renderContext->draw(crb);
					}

					for (auto it : m_gatheredView.renderables)
					{
						IEntityRenderer* entityRenderer = it.first;
						const GatherView::Renderable& r = it.second;

						const AlignedVector< Object* >& objects = staticOnly ? r.staticOnlyObjects : r.objects;
						entityRenderer->build(wc, shadowRenderView, shadowPass, objects);
					}
				});
			}

			// Expose slice data to shaders.
			auto* lsd = lightShaderData + m_gatheredView.lights.size() - 1;
			for (int32_t slice = 0; slice < shadowSettings.cascadingSlices; ++slice)
			{
				const Matrix44 viewToLightSpace = shadowLightViews[slice] * viewInverse;

				viewToLightSpace.axisX().storeUnaligned(lsd[slice].viewToLight0);
				viewToLightSpace.axisY().storeUnaligned(lsd[slice].viewToLight1);
				viewToLightSpace.axisZ().storeUnaligned(lsd[slice].viewToLight2);
				viewToLightSpace.translation().storeUnaligned(lsd[slice].viewToLight3);

				// Write slice coordinates to shaders.
				Vector4(
					(float)(slice * sliceDim) / shmw,
					0.0f,
					(float)sliceDim / shmw,
					1.0f)
					.storeUnaligned(lsd[slice].atlasTransform);
			}
		}

		for (int32_t lightAtlasIndex : lightAtlasIndices)
		{
			const auto& light = m_gatheredView.lights[lightAtlasIndex];
			const Transform lightTransform = light->getTransform();
			const Vector4 lightPosition = lightTransform.translation().xyz1();
			const Vector4 lightDirection = lightTransform.axisY().xyz0();

			auto* lsd = &lightShaderData[lightAtlasIndex];

			// Calculate shadow map projection.
			Matrix44 shadowLightView;
			Matrix44 shadowLightProjection;
			Frustum shadowFrustum;

			shadowFrustum.buildPerspective(light->getRadius(), 1.0f, 1.0f, light->getFarRange());
			shadowLightProjection = perspectiveLh(light->getRadius(), 1.0f, 1.0f, light->getFarRange());

			Vector4 lightAxisX, lightAxisY, lightAxisZ;
			lightAxisZ = -lightDirection;

			orthogonalFrame(
				lightAxisZ,
				lightAxisY,
				lightAxisX
			);

			shadowLightView = Matrix44(
				lightAxisX,
				lightAxisY,
				lightAxisZ,
				lightPosition);

			shadowLightView = shadowLightView.inverse();

			const Matrix44 viewToLightSpace = shadowLightProjection * shadowLightView * viewInverse;
			viewToLightSpace.axisX().storeUnaligned(lsd->viewToLight0);
			viewToLightSpace.axisY().storeUnaligned(lsd->viewToLight1);
			viewToLightSpace.axisZ().storeUnaligned(lsd->viewToLight2);
			viewToLightSpace.translation().storeUnaligned(lsd->viewToLight3);

			// Calculate size of shadow region based on distance from eye.
			const float distance = (worldRenderView.getEyePosition() - lightPosition).xyz0().length();
			const int32_t denom = (int32_t)std::floor(std::sqrt(distance / 4.0f));
			const int32_t atlasSize = std::max(512 >> std::min(denom, 8), 16);

			Packer::Rectangle atlasRect;
			if (!shadowAtlasPacker->insert(atlasSize, atlasSize, atlasRect))
			{
				state.lightSBuffer->unlock();
				return render::RGTargetSet::Invalid;
			}

			// Write atlas coordinates to shaders.
			Vector4(
				(float)(atlasOffset + atlasRect.x) / shmw,
				(float)atlasRect.y / shmh,
				(float)atlasRect.width / shmw,
				(float)atlasRect.height / shmh)
				.storeUnaligned(lsd->atlasTransform);

			addShadowTile(
				{ atlasOffset + atlasRect.x, atlasRect.y, atlasRect.width, atlasRect.height },
				shadowLightProjection,
				shadowLightView,
				shadowFrustum);
		}

		for (int32_t i = 0; i < (int32_t)lightPointIndices.size(); ++i)
		{
			const int32_t lightPointIndex = lightPointIndices[i];
			const auto& light = m_gatheredView.lights[lightPointIndex];
			const Matrix44 lightTransform = light->getTransform().toMatrix44();
			const Vector4 lightPosition = lightTransform.translation().xyz1();

			// Calculate size of each cube face based on distance from eye; faces are packed 3x2 in atlas.
			const float distance = (worldRenderView.getEyePosition() - lightPosition).xyz0().length();
			const int32_t denom = (int32_t)std::floor(std::sqrt(distance / 4.0f));
			int32_t faceSize = std::max(256 >> std::min(denom, 8), 16);

			// Reduce face size until it fits in atlas.
			Packer::Rectangle atlasRect;
			bool inserted = false;
			for (; faceSize >= 16; faceSize >>= 1)
			{
				if ((inserted = shadowAtlasPacker->insert(faceSize * 3, faceSize * 2, atlasRect)) == true)
					break;
			}
			if (!inserted)
				continue;

			const int32_t faceIndex = (int32_t)m_gatheredView.lights.size() + 3 + i * 6;

			// Light axes in view space; used by shaders to select cube face.
			auto* lsd = &lightShaderData[lightPointIndex];
			const Matrix44 lightToView = view * lightTransform;
			lsd->shadowIndex = (float)faceIndex;
			lightToView.axisX().storeUnaligned(lsd->viewToLight0);
			lightToView.axisY().storeUnaligned(lsd->viewToLight1);
			lightToView.axisZ().storeUnaligned(lsd->viewToLight2);

			// Widen field of view so the filter kernel never samples outside of face.
			const float fov = 2.0f * std::atan((float)faceSize / (faceSize - 2 * c_pointShadowFaceBorder));
			const Matrix44 shadowLightProjection = perspectiveLh(fov, 1.0f, c_pointShadowNearZ, light->getFarRange());

			Frustum shadowFrustum;
			shadowFrustum.buildPerspective(fov, 1.0f, c_pointShadowNearZ, light->getFarRange());

			// Faces in order +X, -X, +Y, -Y, +Z, -Z of light space.
			const Vector4 lightAxes[] = { lightTransform.axisX(), lightTransform.axisY(), lightTransform.axisZ() };
			for (int32_t face = 0; face < 6; ++face)
			{
				const Vector4 faceAxisZ = (face & 1) ? -lightAxes[face >> 1] : lightAxes[face >> 1];
				Vector4 faceAxisX, faceAxisY;
				orthogonalFrame(faceAxisZ, faceAxisY, faceAxisX);

				const Matrix44 shadowLightView = Matrix44(
					faceAxisX,
					faceAxisY,
					faceAxisZ,
					lightPosition).inverse();

				auto* fsd = &lightShaderData[faceIndex + face];
				std::memset(fsd, 0, sizeof(LightShaderData));

				const Matrix44 viewToLightSpace = shadowLightProjection * shadowLightView * viewInverse;
				viewToLightSpace.axisX().storeUnaligned(fsd->viewToLight0);
				viewToLightSpace.axisY().storeUnaligned(fsd->viewToLight1);
				viewToLightSpace.axisZ().storeUnaligned(fsd->viewToLight2);
				viewToLightSpace.translation().storeUnaligned(fsd->viewToLight3);

				const Packer::Rectangle tile = {
					atlasOffset + atlasRect.x + (face % 3) * faceSize,
					atlasRect.y + (face / 3) * faceSize,
					faceSize,
					faceSize
				};

				// Write atlas coordinates to shaders.
				Vector4(
					(float)tile.x / shmw,
					(float)tile.y / shmh,
					(float)tile.width / shmw,
					(float)tile.height / shmh)
					.storeUnaligned(fsd->atlasTransform);

				addShadowTile(tile, shadowLightProjection, shadowLightView, shadowFrustum);
			}
		}

		renderGraph.addPass(rp);
	}

	state.lightSBuffer->unlock();
	return shadowMapAtlasTargetSetId;
}

}
