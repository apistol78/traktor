#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Core/Math/Float.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/StructBuffer.h"
#include "Render/StructElement.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image/ImageProcess.h"
#include "Render/Image/ImageProcessData.h"
#include "Render/Image/ImageProcessTargetPool.h"
#include "Resource/IResourceManager.h"
#include "World/IrradianceGrid.h"
#include "World/WorldContext.h"
#include "World/Deferred/LightRendererDeferred.h"
#include "World/Deferred/WorldRendererDeferred.h"
#include "World/Deferred/WorldRenderPassDeferred.h"
#include "World/Entity/GroupEntity.h"
#include "World/SMProj/UniformShadowProjection.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

const int32_t c_maxLightCount = 1024;

const resource::Id< render::ImageProcessData > c_colorTargetCopy(L"{7DCC28A2-C357-B54F-ACF4-8159301B1764}");
const resource::Id< render::ImageProcessData > c_ambientOcclusionLow(L"{ED4F221C-BAB1-4645-BD08-84C5B3FA7C20}");
const resource::Id< render::ImageProcessData > c_ambientOcclusionMedium(L"{A4249C8A-9A0D-B349-B0ED-E8B354CD7BDF}");
const resource::Id< render::ImageProcessData > c_ambientOcclusionHigh(L"{37F82A38-D632-5541-9B29-E77C2F74B0C0}");
const resource::Id< render::ImageProcessData > c_ambientOcclusionUltra(L"{C1C9DDCB-2F82-A94C-BF65-653D8E68F628}");
const resource::Id< render::ImageProcessData > c_antiAliasNone(L"{960283DC-7AC2-804B-901F-8AD4C205F4E0}");
const resource::Id< render::ImageProcessData > c_antiAliasLow(L"{DBF2FBB9-1310-A24E-B443-AF0D018571F7}");
const resource::Id< render::ImageProcessData > c_antiAliasMedium(L"{3E1D810B-339A-F742-9345-4ECA00220D57}");
const resource::Id< render::ImageProcessData > c_antiAliasHigh(L"{0C288028-7BFD-BE46-A25F-F3910BE50319}");
const resource::Id< render::ImageProcessData > c_antiAliasUltra(L"{4750DA97-67F4-E247-A9C2-B4883B1158B2}");
const resource::Id< render::ImageProcessData > c_gammaCorrection(L"{AB0ABBA7-77BF-0A4E-8E3B-4987B801CE6B}");
const resource::Id< render::ImageProcessData > c_motionBlurPrime(L"{73C2C7DC-BD77-F348-A6B7-06E0EFB633D9}");
const resource::Id< render::ImageProcessData > c_motionBlurLow(L"{BDFEFBE0-C5E9-2643-B445-DB02AC5C7687}");
const resource::Id< render::ImageProcessData > c_motionBlurMedium(L"{A70CBA02-B75A-E246-A9B6-99B8B2B98D2A}");
const resource::Id< render::ImageProcessData > c_motionBlurHigh(L"{E893B98C-90A3-9848-B4F3-3D8C0CE57CE8}");
const resource::Id< render::ImageProcessData > c_motionBlurUltra(L"{CD4A0939-233B-2E43-988D-DA6E0DB7A6E6}");
const resource::Id< render::ImageProcessData > c_toneMapFixed(L"{838922A0-49CE-6645-8A9C-BA0E71081033}");
const resource::Id< render::ImageProcessData > c_toneMapAdaptive(L"{BC4FA128-A976-4023-A422-637581ADFD7E}");

// Techniques
const render::Handle s_techniqueDeferredColor(L"World_DeferredColor");
const render::Handle s_techniqueDeferredGBufferWrite(L"World_DeferredGBufferWrite");
const render::Handle s_techniqueReflectionWrite(L"World_ReflectionWrite");
const render::Handle s_techniqueIrradianceWrite(L"World_IrradianceWrite");
const render::Handle s_techniqueVelocityWrite(L"World_VelocityWrite");
const render::Handle s_techniqueShadow(L"World_ShadowWrite");

// Global parameters.
const render::Handle s_handleTime(L"World_Time");
const render::Handle s_handleView(L"World_View");
const render::Handle s_handleViewInverse(L"World_ViewInverse");
const render::Handle s_handleProjection(L"World_Projection");
const render::Handle s_handleColorMap(L"World_ColorMap");
const render::Handle s_handleOcclusionMap(L"World_OcclusionMap");
const render::Handle s_handleDepthMap(L"World_DepthMap");
const render::Handle s_handleLightMap(L"World_LightMap");
const render::Handle s_handleNormalMap(L"World_NormalMap");
const render::Handle s_handleMiscMap(L"World_MiscMap");
const render::Handle s_handleReflectionMap(L"World_ReflectionMap");
const render::Handle s_handleFogDistanceAndDensity(L"World_FogDistanceAndDensity");
const render::Handle s_handleFogColor(L"World_FogColor");
const render::Handle s_handleLightCount(L"World_LightCount");
const render::Handle s_handleLightSBuffer(L"World_LightSBuffer");
const render::Handle s_handleTileSBuffer(L"World_TileSBuffer");
const render::Handle s_handleIrradianceGridSize(L"World_IrradianceGridSize");
const render::Handle s_handleIrradianceGridSBuffer(L"World_IrradianceGridSBuffer");
const render::Handle s_handleIrradianceGridBoundsMin(L"World_IrradianceGridBoundsMin");
const render::Handle s_handleIrradianceGridBoundsMax(L"World_IrradianceGridBoundsMax");

// Render graph.
const render::Handle s_handleGBuffer(L"GBuffer");
const render::Handle s_handleAmbientOcclusion(L"AmbientOcclusion");
const render::Handle s_handleVelocity(L"Velocity");
const render::Handle s_handleColorReadBack(L"ColorReadBack");
const render::Handle s_handleReflections(L"Reflections");
const render::Handle s_handleShadowMapCascade(L"ShadowMapCascade");
const render::Handle s_handleShadowMapAtlas(L"ShadowMapAtlas");
const render::Handle s_handleShadowMask(L"ShadowMask");
const render::Handle s_handleVisual[] =
{
	render::Handle(L"Visual1"),
	render::Handle(L"Visual2"),
	render::Handle(L"Visual3"),
	render::Handle(L"Visual4"),
	render::Handle(L"Visual5"),
	render::Handle(L"Visual6"),
	render::Handle(L"Visual7")
};

resource::Id< render::ImageProcessData > getAmbientOcclusionId(Quality quality)
{
	switch (quality)
	{
	default:
	case QuDisabled:
		return resource::Id< render::ImageProcessData >();
	case QuLow:
		return c_ambientOcclusionLow;
	case QuMedium:
		return c_ambientOcclusionMedium;
	case QuHigh:
		return c_ambientOcclusionHigh;
	case QuUltra:
		return c_ambientOcclusionUltra;
	}
}

resource::Id< render::ImageProcessData > getAntiAliasId(Quality quality)
{
	switch (quality)
	{
	default:
	case QuDisabled:
		return c_antiAliasNone;
	case QuLow:
		return c_antiAliasLow;
	case QuMedium:
		return c_antiAliasMedium;
	case QuHigh:
		return c_antiAliasHigh;
	case QuUltra:
		return c_antiAliasUltra;
	}
}

resource::Id< render::ImageProcessData > getMotionBlurId(Quality quality)
{
	switch (quality)
	{
	default:
	case QuDisabled:
		return resource::Id< render::ImageProcessData >();
	case QuLow:
		return c_motionBlurLow;
	case QuMedium:
		return c_motionBlurMedium;
	case QuHigh:
		return c_motionBlurHigh;
	case QuUltra:
		return c_motionBlurUltra;
	}
}

resource::Id< render::ImageProcessData > getToneMapId(WorldRenderSettings::ExposureMode exposureMode)
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
struct TileShaderData
{
	float lights[4];
	float lightCount[4];
};
#pragma pack()

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRendererDeferred", 0, WorldRendererDeferred, IWorldRenderer)

WorldRendererDeferred::WorldRendererDeferred()
:	m_toneMapQuality(QuDisabled)
,	m_motionBlurQuality(QuDisabled)
,	m_shadowsQuality(QuDisabled)
,	m_ambientOcclusionQuality(QuDisabled)
,	m_antiAliasQuality(QuDisabled)
{
}

bool WorldRendererDeferred::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const WorldCreateDesc& desc
)
{
	render::RenderTargetSetCreateDesc rtscd;
	render::RenderTargetAutoSize rtas;
	render::Clear clear;

	// Store settings.
	m_settings = *desc.worldRenderSettings;
	m_toneMapQuality = desc.toneMapQuality;
	m_motionBlurQuality = desc.motionBlurQuality;
	m_shadowSettings = m_settings.shadowSettings[desc.shadowsQuality];
	m_shadowsQuality = desc.shadowsQuality;
	m_reflectionsQuality = desc.reflectionsQuality;
	m_ambientOcclusionQuality = desc.ambientOcclusionQuality;
	m_antiAliasQuality = desc.antiAliasQuality;

	// Allocate frames.
	m_frames.resize(desc.frameCount);

	// Pack fog parameters.
	m_fogDistanceAndDensity = Vector4(
		m_settings.fogDistanceY,
		m_settings.fogDistanceZ,
		m_settings.fogDensityY,
		m_settings.fogDensityZ
	);
	m_fogColor = m_settings.fogColor;

//	// Create "gbuffer" targets.
//	{
//		render::RenderTargetSetCreateDesc rtscd;
//		rtscd.count = 4;
//		rtscd.width = desc.width;
//		rtscd.height = desc.height;
//		rtscd.multiSample = 0;
//		rtscd.createDepthStencil = false;
//		rtscd.usingPrimaryDepthStencil = (desc.sharedDepthStencil == nullptr) ? true : false;
//		rtscd.sharedDepthStencil = desc.sharedDepthStencil;
//		rtscd.preferTiled = true;
//		rtscd.storeDepthStencil = true;					// Store depth for z-cull later.
//		rtscd.targets[0].format = render::TfR16F;		// Depth (R)
//		rtscd.targets[1].format = render::TfR16G16F;	// Normals (RG)
//		rtscd.targets[2].format = render::TfR11G11B10F;	// Metalness (R), Roughness (G), Specular (B)
//		rtscd.targets[3].format = render::TfR11G11B10F;	// Surface color (RGB)
//
//		m_gbufferTargetSet = renderSystem->createRenderTargetSet(rtscd, T_FILE_LINE_W);
//		if (!m_gbufferTargetSet)
//		{
//			log::error << L"Unable to create depth render target." << Endl;
//			return false;
//		}
//	}
//
//	// Create "ambient occlusion" target.
//	{
//		render::RenderTargetSetCreateDesc rtscd;
//		rtscd.count = 1;
//		rtscd.width = desc.width;
//		rtscd.height = desc.height;
//		rtscd.multiSample = 0;
//		rtscd.createDepthStencil = false;
//		rtscd.usingPrimaryDepthStencil = (desc.sharedDepthStencil == nullptr) ? true : false;
//		rtscd.sharedDepthStencil = desc.sharedDepthStencil;
//		rtscd.preferTiled = true;
//		rtscd.storeDepthStencil = true;
//		rtscd.targets[0].format = render::TfR8;			// Ambient occlusion (R)
//
//		m_ambientOcclusionTargetSet = renderSystem->createRenderTargetSet(rtscd, T_FILE_LINE_W);
//		if (!m_ambientOcclusionTargetSet)
//		{
//			log::error << L"Unable to create ambient occlusion render target." << Endl;
//			return false;
//		}
//	}
//
//	// Create "velocity" target.
//	if (m_motionBlurQuality > QuDisabled)
//	{
//		render::RenderTargetSetCreateDesc rtscd;
//		rtscd.count = 1;
//		rtscd.width = desc.width;
//		rtscd.height = desc.height;
//		rtscd.multiSample = 0;
//		rtscd.createDepthStencil = false;
//		rtscd.usingPrimaryDepthStencil = (desc.sharedDepthStencil == nullptr) ? true : false;
//		rtscd.sharedDepthStencil = desc.sharedDepthStencil;
//		rtscd.preferTiled = true;
//		rtscd.storeDepthStencil = false;
//		rtscd.targets[0].format = render::TfR16G16F;
//
//		m_velocityTargetSet = renderSystem->createRenderTargetSet(rtscd, T_FILE_LINE_W);
//		if (!m_velocityTargetSet)
//		{
//			log::error << L"Unable to create velocity render target; motion blur disabled." << Endl;
//			m_motionBlurQuality = QuDisabled;
//		}
//	}
//
//	// Create "color read-back" target.
//	{
//		render::RenderTargetSetCreateDesc rtscd;
//		rtscd.count = 1;
//		rtscd.width = previousLog2(desc.width);
//		rtscd.height = previousLog2(desc.height);
//		rtscd.multiSample = 0;
//		rtscd.createDepthStencil = false;
//		rtscd.usingPrimaryDepthStencil = false;
//		rtscd.preferTiled = true;
//		rtscd.storeDepthStencil = false;
//		rtscd.ignoreStencil = true;
//		rtscd.generateMips = true;
//#if !defined(__ANDROID__) && !defined(__IOS__)
//		rtscd.targets[0].format = render::TfR16G16B16A16F;
//#else
//		rtscd.targets[0].format = render::TfR11G11B10F;
//#endif
//
//		m_colorTargetSet = renderSystem->createRenderTargetSet(rtscd, T_FILE_LINE_W);
//		if (!m_colorTargetSet)
//		{
//			log::error << L"Unable to create color read-back render target." << Endl;
//			return false;
//		}
//	}
//
//	// Create "reflections" target.
//	if (m_reflectionsQuality > QuDisabled)
//	{
//		render::RenderTargetSetCreateDesc rtscd;
//		rtscd.count = 1;
//#if !defined(__ANDROID__) && !defined(__IOS__)
//		rtscd.width = desc.width;
//		rtscd.height = desc.height;
//#else
//		rtscd.width = desc.width / 2;
//		rtscd.height = desc.height / 2;
//#endif
//		rtscd.multiSample = 0;
//		rtscd.createDepthStencil = false;
//		rtscd.usingPrimaryDepthStencil = false;
//		rtscd.preferTiled = true;
//		rtscd.storeDepthStencil = false;
//		rtscd.ignoreStencil = true;
//		rtscd.generateMips = false;
//#if !defined(__ANDROID__) && !defined(__IOS__)
//		rtscd.targets[0].format = render::TfR16G16B16A16F;
//#else
//		rtscd.targets[0].format = render::TfR11G11B10F;
//#endif
//
//		m_reflectionsTargetSet = renderSystem->createRenderTargetSet(rtscd, T_FILE_LINE_W);
//		if (!m_reflectionsTargetSet)
//		{
//			log::error << L"Unable to create reflections render target." << Endl;
//			return false;
//		}
//	}
//
//	// Create "shadow map" targets.
//	if (m_shadowsQuality > QuDisabled)
//	{
//		render::RenderSystemInformation info;
//		renderSystem->getInformation(info);
//
//		int32_t maxResolution = m_shadowSettings.resolution;
//		if (info.dedicatedMemoryTotal < 512 * 1024 * 1024)
//			maxResolution /= 2;
//
//		int32_t resolution = min< int32_t >(
//			nearestLog2((max< int32_t >(desc.width, desc.height) * 190) / 100),
//			maxResolution
//		);
//		T_DEBUG(L"Using shadow map resolution " << resolution);
//
//		// Create shadow screen projection processes.
//		resource::Proxy< render::ImageProcessData > shadowMaskProject;
//		if (!resourceManager->bind(m_shadowSettings.maskProject, shadowMaskProject))
//		{
//			log::warning << L"Unable to create shadow project process; shadows disabled." << Endl;
//			m_shadowsQuality = QuDisabled;
//		}
//
//		if (m_shadowsQuality > QuDisabled)
//		{
//			m_shadowMaskProject = new render::ImageProcess();
//			if (!m_shadowMaskProject->create(
//				shadowMaskProject,
//				nullptr,
//				resourceManager,
//				renderSystem,
//				desc.width / m_shadowSettings.maskDenominator,
//				desc.height / m_shadowSettings.maskDenominator,
//				desc.allTargetsPersistent
//			))
//			{
//				log::warning << L"Unable to create shadow project process; shadows disabled." << Endl;
//				m_shadowsQuality = QuDisabled;
//			}
//		}
//
//		// Create shadow render targets.
//		if (m_shadowsQuality > QuDisabled)
//		{
//			render::RenderTargetSetCreateDesc rtscd;
//
//			// Create shadow cascade map target.
//			rtscd.count = 0;
//			rtscd.width = resolution;
//			rtscd.height = resolution;
//			rtscd.multiSample = 0;
//			rtscd.createDepthStencil = true;
//			rtscd.usingDepthStencilAsTexture = true;
//			rtscd.usingPrimaryDepthStencil = false;
//			rtscd.ignoreStencil = true;
//			rtscd.preferTiled = true;
//			rtscd.storeDepthStencil = true;	// Used as texture.
//			if ((m_shadowCascadeTargetSet = renderSystem->createRenderTargetSet(rtscd, T_FILE_LINE_W)) == nullptr)
//				m_shadowsQuality = QuDisabled;
//
//			// Create shadow screen mask map target.
//			rtscd.count = 1;
//			rtscd.width = desc.width / m_shadowSettings.maskDenominator;
//			rtscd.height = desc.height / m_shadowSettings.maskDenominator;
//			rtscd.multiSample = 0;
//			rtscd.createDepthStencil = false;
//			rtscd.usingDepthStencilAsTexture = false;
//			rtscd.usingPrimaryDepthStencil = false;
//			rtscd.ignoreStencil = true;
//			rtscd.preferTiled = true;
//			rtscd.storeDepthStencil = false;
//			rtscd.targets[0].format = render::TfR8;
//			rtscd.targets[0].sRGB = false;
//			if ((m_shadowMaskTargetSet = renderSystem->createRenderTargetSet(rtscd, T_FILE_LINE_W)) == nullptr)
//				m_shadowsQuality = QuDisabled;
//
//			// Create shadow atlas map target.
//			rtscd.count = 0;
//#if !defined(__ANDROID__) && !defined(__IOS__)
//			rtscd.width =
//			rtscd.height = 4096;
//#else
//			rtscd.width =
//			rtscd.height = 1024;
//#endif
//			rtscd.multiSample = 0;
//			rtscd.createDepthStencil = true;
//			rtscd.usingDepthStencilAsTexture = true;
//			rtscd.usingPrimaryDepthStencil = false;
//			rtscd.ignoreStencil = true;
//			rtscd.preferTiled = true;
//			rtscd.storeDepthStencil = true;	// Used as texture.
//			if ((m_shadowAtlasTargetSet = renderSystem->createRenderTargetSet(rtscd, T_FILE_LINE_W)) == nullptr)
//				m_shadowsQuality = QuDisabled;
//		}
//
//		// Ensure targets are destroyed if something went wrong in setup.
//		if (m_shadowsQuality == QuDisabled)
//		{
//			safeDestroy(m_shadowCascadeTargetSet);
//			safeDestroy(m_shadowMaskTargetSet);
//			safeDestroy(m_shadowAtlasTargetSet);
//		}
//	}
//
//	// Create "visual" and "intermediate" target.
//	{
//		render::RenderTargetSetCreateDesc rtscd;
//		rtscd.count = 1;
//		rtscd.width = desc.width;
//		rtscd.height = desc.height;
//		rtscd.multiSample = desc.multiSample;
//		rtscd.createDepthStencil = false;
//		rtscd.usingPrimaryDepthStencil = (desc.sharedDepthStencil == nullptr) ? true : false;
//		rtscd.sharedDepthStencil = desc.sharedDepthStencil;
//		rtscd.preferTiled = true;
//		rtscd.storeDepthStencil = false;
//#if !defined(__ANDROID__) && !defined(__IOS__)
//		rtscd.targets[0].format = render::TfR32G32B32A32F;
//#else
//		rtscd.targets[0].format = render::TfR11G11B10F;
//#endif
//
//		m_visualTargetSet = renderSystem->createRenderTargetSet(rtscd, T_FILE_LINE_W);
//		if (!m_visualTargetSet)
//			return false;
//
//		m_intermediateTargetSet = renderSystem->createRenderTargetSet(rtscd, T_FILE_LINE_W);
//		if (!m_intermediateTargetSet)
//			return false;
//	}

	// Create "color read-back" copy processing.
	{
		resource::Proxy< render::ImageProcessData > colorTargetCopy;

		if (!resourceManager->bind(c_colorTargetCopy, colorTargetCopy))
			log::warning << L"Unable to create color read-back processing; color read-back disabled." << Endl;

		if (colorTargetCopy)
		{
			m_colorTargetCopy = new render::ImageProcess();
			if (!m_colorTargetCopy->create(
				colorTargetCopy,
				nullptr,
				resourceManager,
				renderSystem,
				desc.width,
				desc.height,
				desc.allTargetsPersistent
			))
			{
				log::warning << L"Unable to create color read-back processing; color read-back disabled." << Endl;
				m_colorTargetCopy = nullptr;
			}
		}
	}

	// Create ambient occlusion processing.
	{
		resource::Id< render::ImageProcessData > ambientOcclusionId = getAmbientOcclusionId(m_ambientOcclusionQuality);
		resource::Proxy< render::ImageProcessData > ambientOcclusion;

		if (ambientOcclusionId)
		{
			if (!resourceManager->bind(ambientOcclusionId, ambientOcclusion))
				log::warning << L"Unable to create ambient occlusion process; AO disabled." << Endl;
		}

		if (ambientOcclusion)
		{
			m_ambientOcclusion = new render::ImageProcess();
			if (!m_ambientOcclusion->create(
				ambientOcclusion,
				nullptr,
				resourceManager,
				renderSystem,
				desc.width,
				desc.height,
				desc.allTargetsPersistent
			))
			{
				log::warning << L"Unable to create ambient occlusion process; AO disabled." << Endl;
				m_ambientOcclusion = nullptr;
			}
		}
	}

	// Create antialias processing.
	{
		resource::Id< render::ImageProcessData > antiAliasId = getAntiAliasId(m_antiAliasQuality);
		resource::Proxy< render::ImageProcessData > antiAlias;

		if (antiAliasId != c_antiAliasNone)
		{
			if (!resourceManager->bind(antiAliasId, antiAlias))
				log::warning << L"Unable to create antialias process; AA disabled." << Endl;
		}

		if (antiAlias)
		{
			m_antiAlias = new render::ImageProcess();
			if (!m_antiAlias->create(
				antiAlias,
				nullptr,
				resourceManager,
				renderSystem,
				desc.width,
				desc.height,
				desc.allTargetsPersistent
			))
			{
				log::warning << L"Unable to create antialias process; AA disabled." << Endl;
				m_antiAlias = nullptr;
			}
		}
	}

	// Create "visual" post processing filter.
	{
		const resource::Id< render::ImageProcessData >& imageProcessSettings = desc.worldRenderSettings->imageProcess[desc.imageProcessQuality];
		if (imageProcessSettings)
		{
			resource::Proxy< render::ImageProcessData > imageProcess;
			if (!resourceManager->bind(imageProcessSettings, imageProcess))
				log::warning << L"Unable to create visual post processing image filter; post processing disabled." << Endl;

			if (imageProcess)
			{
				m_visualImageProcess = new render::ImageProcess();
				if (!m_visualImageProcess->create(
					imageProcess,
					nullptr,
					resourceManager,
					renderSystem,
					desc.width,
					desc.height,
					desc.allTargetsPersistent
				))
				{
					log::warning << L"Unable to create visual post processing; post processing disabled." << Endl;
					m_visualImageProcess = nullptr;
				}
			}
		}
	}

	// Create gamma correction processing.
	if (
		m_settings.linearLighting &&
		std::abs(desc.gamma - 1.0f) > FUZZY_EPSILON
	)
	{
		resource::Proxy< render::ImageProcessData > gammaCorrection;
		if (!resourceManager->bind(c_gammaCorrection, gammaCorrection))
			log::warning << L"Unable to create gamma correction process; gamma correction disabled." << Endl;

		if (gammaCorrection)
		{
			m_gammaCorrectionImageProcess = new render::ImageProcess();
			if (m_gammaCorrectionImageProcess->create(
				gammaCorrection,
				nullptr,
				resourceManager,
				renderSystem,
				desc.width,
				desc.height,
				desc.allTargetsPersistent
			))
			{
				m_gammaCorrectionImageProcess->setFloatParameter(render::getParameterHandle(L"World_Gamma"), desc.gamma);
				m_gammaCorrectionImageProcess->setFloatParameter(render::getParameterHandle(L"World_GammaInverse"), 1.0f / desc.gamma);
			}
			else
			{
				log::warning << L"Unable to create gamma correction process; gamma correction disabled." << Endl;
				m_gammaCorrectionImageProcess = nullptr;
			}
		}
	}

	// Create motion blur prime processing.
	if (m_motionBlurQuality > QuDisabled)
	{
		resource::Proxy< render::ImageProcessData > motionBlurPrime;
		if (!resourceManager->bind(c_motionBlurPrime, motionBlurPrime))
		{
			log::warning << L"Unable to create motion blur prime process; motion blur disabled." << Endl;
			m_motionBlurQuality = QuDisabled;
		}

		if (motionBlurPrime)
		{
			m_motionBlurPrimeImageProcess = new render::ImageProcess();
			if (!m_motionBlurPrimeImageProcess->create(
				motionBlurPrime,
				nullptr,
				resourceManager,
				renderSystem,
				desc.width,
				desc.height,
				desc.allTargetsPersistent
			))
			{
				log::warning << L"Unable to create motion blur process; motion blur disabled." << Endl;
				m_motionBlurPrimeImageProcess = nullptr;
				m_motionBlurQuality = QuDisabled;
			}
		}
	}

	// Create motion blur final processing.
	if (m_motionBlurQuality > QuDisabled)
	{
		resource::Id< render::ImageProcessData > motionBlurId = getMotionBlurId(desc.motionBlurQuality);
		resource::Proxy< render::ImageProcessData > motionBlur;

		if (!resourceManager->bind(motionBlurId, motionBlur))
		{
			log::warning << L"Unable to create motion blur process; motion blur disabled." << Endl;
			m_motionBlurQuality = QuDisabled;
		}

		if (motionBlur)
		{
			m_motionBlurImageProcess = new render::ImageProcess();
			if (!m_motionBlurImageProcess->create(
				motionBlur,
				nullptr,
				resourceManager,
				renderSystem,
				desc.width,
				desc.height,
				desc.allTargetsPersistent
			))
			{
				log::warning << L"Unable to create motion blur process; motion blur disabled." << Endl;
				m_motionBlurImageProcess = nullptr;
				m_motionBlurQuality = QuDisabled;
			}
		}
	}

	// Create tone map processing.
	if (m_toneMapQuality > QuDisabled)
	{
		resource::Id< render::ImageProcessData > toneMapId = getToneMapId(m_settings.exposureMode);
		resource::Proxy< render::ImageProcessData > toneMap;

		if (!resourceManager->bind(toneMapId, toneMap))
		{
			log::warning << L"Unable to create tone map process." << Endl;
			m_toneMapQuality = QuDisabled;
		}

		if (toneMap)
		{
			m_toneMapImageProcess = new render::ImageProcess();
			if (m_toneMapImageProcess->create(
				toneMap,
				nullptr,
				resourceManager,
				renderSystem,
				desc.width,
				desc.height,
				desc.allTargetsPersistent
			))
				m_toneMapImageProcess->setFloatParameter(
					render::getParameterHandle(L"World_Exposure"),
					m_settings.exposure
				);
			else
			{
				log::warning << L"Unable to create tone map process; tone mapping disabled." << Endl;
				m_toneMapImageProcess = nullptr;
				m_toneMapQuality = QuDisabled;
			}
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

		AlignedVector< render::StructElement > tileShaderDataStruct;
		tileShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(TileShaderData, lights)));
		tileShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(TileShaderData, lightCount)));
		T_FATAL_ASSERT(sizeof(TileShaderData) == render::getStructSize(tileShaderDataStruct));

		frame.tileSBuffer = renderSystem->createStructBuffer(
			tileShaderDataStruct,
			render::getStructSize(tileShaderDataStruct) * 16 * 16
		);
		if (!frame.tileSBuffer)
			return false;
	}

	// Create light primitive renderer.
	m_lightRenderer = new LightRendererDeferred();
	if (!m_lightRenderer->create(
		resourceManager,
		renderSystem
	))
	{
		log::error << L"Unable to create light primitive renderer." << Endl;
		return false;
	}

	// Create irradiance grid.
	if (!m_settings.irradianceGrid.isNull())
	{
		if (!resourceManager->bind(m_settings.irradianceGrid, m_irradianceGrid))
			return false;
	}

	// Determine slice distances.
	for (int32_t i = 0; i < m_shadowSettings.cascadingSlices; ++i)
	{
		float ii = float(i) / m_shadowSettings.cascadingSlices;
		float log = powf(ii, m_shadowSettings.cascadingLambda);
		m_slicePositions[i] = lerp(m_settings.viewNearZ, m_shadowSettings.farZ, log);
	}
	m_slicePositions[m_shadowSettings.cascadingSlices] = m_shadowSettings.farZ;

	m_entityRenderers = desc.entityRenderers;
	m_rootEntity = new GroupEntity();

	// Create render graph.
	m_renderGraph = new render::RenderGraph(
		renderSystem,
		desc.width,
		desc.height
	);
	
	// GBuffer
	rtscd.count = 4;
	rtscd.multiSample = 0;
	rtscd.createDepthStencil = false;
	rtscd.usingPrimaryDepthStencil = (desc.sharedDepthStencil == nullptr) ? true : false;
	rtscd.sharedDepthStencil = desc.sharedDepthStencil;
	rtscd.preferTiled = true;
	rtscd.storeDepthStencil = true;					// Store depth for z-cull later.
	rtscd.targets[0].format = render::TfR16F;		// Depth (R)
	rtscd.targets[1].format = render::TfR16G16F;	// Normals (RG)
	rtscd.targets[2].format = render::TfR11G11B10F;	// Metalness (R), Roughness (G), Specular (B)
	rtscd.targets[3].format = render::TfR11G11B10F;	// Surface color (RGB)
	rtas.screenWidthDenom = 1;
	rtas.screenHeightDenom = 1;
	m_renderGraph->addRenderTarget(L"GBuffer", s_handleGBuffer, rtscd, rtas);

	// Ambient occlusion.
	rtscd.count = 1;
	rtscd.multiSample = 0;
	rtscd.createDepthStencil = false;
	rtscd.usingPrimaryDepthStencil = (desc.sharedDepthStencil == nullptr) ? true : false;
	rtscd.sharedDepthStencil = desc.sharedDepthStencil;
	rtscd.preferTiled = true;
	rtscd.storeDepthStencil = true;
	rtscd.targets[0].format = render::TfR8;			// Ambient occlusion (R)
	rtas.screenWidthDenom = 1;
	rtas.screenHeightDenom = 1;
	m_renderGraph->addRenderTarget(L"Ambient Occlusion", s_handleAmbientOcclusion, rtscd, rtas);

	// Velocity
	rtscd.count = 1;
	rtscd.multiSample = 0;
	rtscd.createDepthStencil = false;
	rtscd.usingPrimaryDepthStencil = (desc.sharedDepthStencil == nullptr) ? true : false;
	rtscd.sharedDepthStencil = desc.sharedDepthStencil;
	rtscd.preferTiled = true;
	rtscd.storeDepthStencil = false;
	rtscd.targets[0].format = render::TfR16G16F;
	rtas.screenWidthDenom = 1;
	rtas.screenHeightDenom = 1;
	m_renderGraph->addRenderTarget(L"Velocity", s_handleVelocity, rtscd, rtas);

	// Color read-back.
	rtscd.count = 1;
	rtscd.multiSample = 0;
	rtscd.createDepthStencil = false;
	rtscd.usingPrimaryDepthStencil = (desc.sharedDepthStencil == nullptr) ? true : false;
	rtscd.sharedDepthStencil = desc.sharedDepthStencil;
	rtscd.preferTiled = true;
	rtscd.storeDepthStencil = false;
	rtscd.ignoreStencil = true;
	rtscd.generateMips = true;
#if !defined(__ANDROID__) && !defined(__IOS__)
		rtscd.targets[0].format = render::TfR16G16B16A16F;
#else
		rtscd.targets[0].format = render::TfR11G11B10F;
#endif
	rtas.screenWidthDenom = 2;
	rtas.screenHeightDenom = 2;
	m_renderGraph->addRenderTarget(L"Color ReadBack", s_handleColorReadBack, rtscd, rtas);
	
	// Reflections
	if (m_reflectionsQuality != QuDisabled)
	{
		rtscd.count = 1;
		rtscd.multiSample = 0;
		rtscd.createDepthStencil = false;
		rtscd.usingPrimaryDepthStencil = (desc.sharedDepthStencil == nullptr) ? true : false;
		rtscd.sharedDepthStencil = desc.sharedDepthStencil;
		rtscd.preferTiled = true;
		rtscd.storeDepthStencil = false;
		rtscd.ignoreStencil = true;
#if !defined(__ANDROID__) && !defined(__IOS__)
		rtscd.targets[0].format = render::TfR16G16B16A16F;
		rtas.screenWidthDenom = 1;
		rtas.screenHeightDenom = 1;
#else
		rtscd.targets[0].format = render::TfR11G11B10F;
		rtas.screenWidthDenom = 2;
		rtas.screenHeightDenom = 2;
#endif
		m_renderGraph->addRenderTarget(L"Reflections", s_handleReflections, rtscd, rtas);
	}

	const bool shadowsEnable = (bool)(m_shadowsQuality != QuDisabled);
	if (shadowsEnable)
	{
		// Cascading shadow map.
		rtscd.count = 0;
		rtscd.width = 1024;
		rtscd.height = m_settings.shadowSettings[m_shadowsQuality].cascadingSlices * 1024;
		rtscd.multiSample = 0;
		rtscd.createDepthStencil = true;
		rtscd.usingDepthStencilAsTexture = true;
		rtscd.usingPrimaryDepthStencil = false;
		rtscd.ignoreStencil = true;
		rtscd.preferTiled = true;
		m_renderGraph->addRenderTarget(L"ShadowMap Cascade", s_handleShadowMapCascade, rtscd);

		// Atlas shadow map.
		rtscd.count = 0;
		rtscd.width =
		rtscd.height = 4096;
		rtscd.multiSample = 0;
		rtscd.createDepthStencil = true;
		rtscd.usingDepthStencilAsTexture = true;
		rtscd.usingPrimaryDepthStencil = false;
		rtscd.ignoreStencil = true;
		rtscd.preferTiled = true;
		m_renderGraph->addRenderTarget(L"ShadowMap Atlas", s_handleShadowMapAtlas, rtscd);

		// Screen space shadow mask.
		rtscd.count = 1;
		rtscd.width = 0;
		rtscd.height = 0;
		rtscd.multiSample = 0;
		rtscd.createDepthStencil = false;
		rtscd.usingDepthStencilAsTexture = false;
		rtscd.usingPrimaryDepthStencil = false;
		rtscd.sharedDepthStencil = desc.sharedDepthStencil;
		rtscd.ignoreStencil = true;
		rtscd.preferTiled = true;
		rtscd.storeDepthStencil = false;
		rtscd.targets[0].format = render::TfR8;
		rtas.screenWidthDenom = m_shadowSettings.maskDenominator;
		rtas.screenHeightDenom = m_shadowSettings.maskDenominator;
		m_renderGraph->addRenderTarget(L"ShadowMask", s_handleShadowMask, rtscd, rtas);
	}

	// Visual
	rtscd.count = 1;
	rtscd.multiSample = desc.multiSample;
	rtscd.createDepthStencil = false;
	rtscd.usingPrimaryDepthStencil = (desc.sharedDepthStencil == nullptr) ? true : false;
	rtscd.sharedDepthStencil = desc.sharedDepthStencil;
	rtscd.preferTiled = true;
#if !defined(__ANDROID__) && !defined(__IOS__)
		rtscd.targets[0].format = render::TfR32G32B32A32F;
#else
		rtscd.targets[0].format = render::TfR11G11B10F;
#endif
	rtas.screenWidthDenom = 1;
	rtas.screenHeightDenom = 1;
	for (int32_t i = 0; i < sizeof_array(s_handleVisual); ++i)
		m_renderGraph->addRenderTarget(L"Visual", s_handleVisual[i], rtscd, rtas);

	for (auto& frame : m_frames)
		frame.renderContext = new render::RenderContext(1 * 1024 * 1024);

	return true;
}

void WorldRendererDeferred::destroy()
{
	for (auto& frame : m_frames)
	{
		safeDestroy(frame.lightSBuffer);
		safeDestroy(frame.tileSBuffer);
	}
	m_frames.clear();

	safeDestroy(m_lightRenderer);
	safeDestroy(m_toneMapImageProcess);
	safeDestroy(m_motionBlurPrimeImageProcess);
	safeDestroy(m_motionBlurImageProcess);
	safeDestroy(m_gammaCorrectionImageProcess);
	safeDestroy(m_visualImageProcess);
	safeDestroy(m_antiAlias);
	safeDestroy(m_ambientOcclusion);
	safeDestroy(m_colorTargetCopy);
	safeDestroy(m_shadowMaskProject);

	//safeDestroy(m_shadowAtlasTargetSet);
	//safeDestroy(m_shadowMaskTargetSet);
	//safeDestroy(m_shadowCascadeTargetSet);
	//safeDestroy(m_colorTargetSet);
	//safeDestroy(m_velocityTargetSet);
	//safeDestroy(m_ambientOcclusionTargetSet);
	//safeDestroy(m_gbufferTargetSet);
	//safeDestroy(m_intermediateTargetSet);
	//safeDestroy(m_visualTargetSet);

	//m_shadowProjection = nullptr;

	safeDestroy(m_renderGraph);

	m_irradianceGrid.clear();
}

void WorldRendererDeferred::attach(Entity* entity)
{
	m_rootEntity->addEntity(entity);
}

void WorldRendererDeferred::build(const WorldRenderView& worldRenderView, int32_t frame)
{
	WorldContext wc(
		m_entityRenderers,
		m_frames[frame].renderContext,
		m_rootEntity
	);

	// Reset render context by flushing it.
	m_frames[frame].renderContext->flush();

	// Gather active lights.
	m_lights.resize(0);
	wc.gather(
		worldRenderView,
		m_rootEntity,
		m_lights
	);

	// Discard excessive lights.
	if (m_lights.size() > c_maxLightCount)
		m_lights.resize(c_maxLightCount);

	// Lock light sbuffer; \tbd data is currently written both when setting
	// up render passes and when executing passes.
	LightShaderData* lightShaderData = (LightShaderData*)m_frames[frame].lightSBuffer->lock();
	TileShaderData* tileShaderData = (TileShaderData*)m_frames[frame].tileSBuffer->lock();
	T_FATAL_ASSERT(lightShaderData != nullptr);

	// \tbd Flush all entity renderers first, only used by probes atm and need to render to targets.
	// Until we have RenderGraph properly implemented we need to make sure
	// rendering probes doesn't nest render passes.
	wc.flush();
	wc.getRenderContext()->merge(render::RpAll);

	buildGBuffer(worldRenderView);
	buildVelocity(worldRenderView);
	buildAmbientOcclusion(worldRenderView);
	buildLights(worldRenderView, frame, lightShaderData, tileShaderData);
	buildShadowMask(worldRenderView);
	buildReflections(worldRenderView);
	buildVisual(worldRenderView, frame);
	buildCopyFrame(worldRenderView);
	buildEndFrame(worldRenderView);

	// Validate render graph.
	if (!m_renderGraph->validate())
		return;

	// Build render context through render graph.
	m_renderGraph->build(
		m_frames[frame].renderContext
	);

	// Unlock light sbuffers.
	m_frames[frame].tileSBuffer->unlock();
	m_frames[frame].lightSBuffer->unlock();

	m_rootEntity->removeAllEntities();
}

void WorldRendererDeferred::render(render::IRenderView* renderView, int32_t frame)
{
	m_frames[frame].renderContext->render(renderView);
}

render::ImageProcess* WorldRendererDeferred::getVisualImageProcess()
{
	return m_visualImageProcess;
}

void WorldRendererDeferred::getDebugTargets(std::vector< render::DebugTarget >& outTargets) const
{
	//if (m_visualTargetSet)
	//	outTargets.push_back(render::DebugTarget(L"Visual", render::DtvDefault, m_visualTargetSet->getColorTexture(0)));

	//if (m_intermediateTargetSet)
	//	outTargets.push_back(render::DebugTarget(L"Intermediate", render::DtvDefault, m_intermediateTargetSet->getColorTexture(0)));

	//if (m_gbufferTargetSet)
	//{
	//	outTargets.push_back(render::DebugTarget(L"GBuffer depth", render::DtvViewDepth, m_gbufferTargetSet->getColorTexture(0)));
	//	outTargets.push_back(render::DebugTarget(L"GBuffer normals", render::DtvNormals, m_gbufferTargetSet->getColorTexture(1)));
	//	outTargets.push_back(render::DebugTarget(L"GBuffer metalness", render::DtvDeferredMetalness, m_gbufferTargetSet->getColorTexture(2)));
	//	outTargets.push_back(render::DebugTarget(L"GBuffer roughness", render::DtvDeferredRoughness, m_gbufferTargetSet->getColorTexture(2)));
	//	outTargets.push_back(render::DebugTarget(L"GBuffer specular", render::DtvDeferredSpecular, m_gbufferTargetSet->getColorTexture(2)));
	//	outTargets.push_back(render::DebugTarget(L"GBuffer surface color", render::DtvDefault, m_gbufferTargetSet->getColorTexture(3)));
	//}

	//if (m_ambientOcclusionTargetSet)
	//	outTargets.push_back(render::DebugTarget(L"Ambient occlusion", render::DtvDefault, m_ambientOcclusionTargetSet->getColorTexture(0)));

	//if (m_velocityTargetSet)
	//	outTargets.push_back(render::DebugTarget(L"Velocity", render::DtvVelocity, m_velocityTargetSet->getColorTexture(0)));

	//if (m_colorTargetSet)
	//	outTargets.push_back(render::DebugTarget(L"Color read-back copy", render::DtvDefault, m_colorTargetSet->getColorTexture(0)));

	//if (m_reflectionsTargetSet)
	//	outTargets.push_back(render::DebugTarget(L"Reflections", render::DtvDefault, m_reflectionsTargetSet->getColorTexture(0)));

	//if (m_shadowCascadeTargetSet)
	//	outTargets.push_back(render::DebugTarget(L"Shadow map (cascade)", render::DtvShadowMap, m_shadowCascadeTargetSet->getDepthTexture()));

	//if (m_shadowMaskTargetSet)
	//	outTargets.push_back(render::DebugTarget(L"Shadow mask (cascade)", render::DtvDefault, m_shadowMaskTargetSet->getColorTexture(0)));

	//if (m_shadowAtlasTargetSet)
	//	outTargets.push_back(render::DebugTarget(L"Shadow map (atlas)", render::DtvShadowMap, m_shadowAtlasTargetSet->getDepthTexture()));

	//if (m_colorTargetCopy)
	//	m_colorTargetCopy->getDebugTargets(outTargets);

	//if (m_ambientOcclusion)
	//	m_ambientOcclusion->getDebugTargets(outTargets);

	//if (m_antiAlias)
	//	m_antiAlias->getDebugTargets(outTargets);

	//if (m_visualImageProcess)
	//	m_visualImageProcess->getDebugTargets(outTargets);

	//if (m_gammaCorrectionImageProcess)
	//	m_gammaCorrectionImageProcess->getDebugTargets(outTargets);

	//if (m_motionBlurPrimeImageProcess)
	//	m_motionBlurPrimeImageProcess->getDebugTargets(outTargets);

	//if (m_motionBlurImageProcess)
	//	m_motionBlurImageProcess->getDebugTargets(outTargets);

	//if (m_toneMapImageProcess)
	//	m_toneMapImageProcess->getDebugTargets(outTargets);
}

void WorldRendererDeferred::buildGBuffer(const WorldRenderView& worldRenderView)
{
	m_renderGraph->addPass(
		L"GBuffer",
		[&](render::RenderPassBuilder& builder)
		{
			const float clearZ = m_settings.viewFarZ;

			render::Clear clear;
			clear.mask = render::CfColor | render::CfDepth;
			clear.colors[0] = Color4f(clearZ, clearZ, clearZ, clearZ);	// depth
			clear.colors[1] = Color4f(0.0f, 0.0f, 1.0f, 0.0f);	// normal
			clear.colors[2] = Color4f(0.0f, 1.0f, 0.0f, 1.0f);	// misc
			clear.colors[3] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);	// surface
			clear.depth = 1.0f;	

			builder.setOutput(s_handleGBuffer, clear);
		},
		[=](render::RenderPassResources& resources, render::RenderContext* renderContext)
		{
			WorldContext wc(
				m_entityRenderers,
				renderContext,
				m_rootEntity
			);

			auto sharedParams = renderContext->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(renderContext);
			sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
			sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
			sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
			sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
			sharedParams->endParameters(renderContext);

			WorldRenderPassDeferred gbufferPass(
				s_techniqueDeferredGBufferWrite,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::PfFirst,
				false
			);

			T_ASSERT(!renderContext->havePendingDraws());
			wc.build(worldRenderView, gbufferPass, m_rootEntity);
			wc.flush(worldRenderView, gbufferPass);
			renderContext->merge(render::RpAll);
		}
	);
}

void WorldRendererDeferred::buildVelocity(const WorldRenderView& worldRenderView)
{
	if (m_motionBlurQuality == QuDisabled)
		return;

	if (m_motionBlurPrimeImageProcess)
	{
		m_renderGraph->addPass(
			L"Velocity",
			[&](render::RenderPassBuilder& builder)
			{
				builder.setOutput(s_handleVelocity);
			},
			[=](render::RenderPassResources& resources, render::RenderContext* renderContext)
			{
				WorldContext wc(
					m_entityRenderers,
					renderContext,
					m_rootEntity
				);

				auto gbufferTargetSet = m_renderGraph->getRenderTarget(s_handleGBuffer);

				render::ImageProcessStep::Instance::RenderParams params;
				params.viewFrustum = worldRenderView.getViewFrustum();
				params.lastView = worldRenderView.getLastView();
				params.view = worldRenderView.getView();
				params.projection = worldRenderView.getProjection();
				params.deltaTime = 0.0f;

				m_motionBlurPrimeImageProcess->build(
					renderContext,
					nullptr,	// color
					gbufferTargetSet->getColorTexture(0),	// depth
					nullptr,	// normal
					nullptr,	// velocity
					nullptr,	// shadow mask
					params
				);

				auto sharedParams = renderContext->alloc< render::ProgramParameters >();
				sharedParams->beginParameters(renderContext);
				sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
				sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
				sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
				sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
				sharedParams->endParameters(renderContext);

				WorldRenderPassDeferred velocityPass(
					s_techniqueVelocityWrite,
					sharedParams,
					worldRenderView,
					IWorldRenderPass::PfNone,
					false
				);

				T_ASSERT(!renderContext->havePendingDraws());
				wc.build(worldRenderView, velocityPass, m_rootEntity);
				wc.flush(worldRenderView, velocityPass);
				renderContext->merge(render::RpAll);
			}
		);
	}
	else
	{
		m_renderGraph->addPass(
			L"Velocity",
			[&](render::RenderPassBuilder& builder)
			{
				render::Clear clear;
				clear.mask = render::CfColor;
				clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
				builder.setOutput(s_handleVelocity, clear);
			},
			[=](render::RenderPassResources& resources, render::RenderContext* renderContext)
			{
				WorldContext wc(
					m_entityRenderers,
					renderContext,
					m_rootEntity
				);

				auto sharedParams = renderContext->alloc< render::ProgramParameters >();
				sharedParams->beginParameters(renderContext);
				sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
				sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
				sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
				sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
				sharedParams->endParameters(renderContext);

				WorldRenderPassDeferred velocityPass(
					s_techniqueVelocityWrite,
					sharedParams,
					worldRenderView,
					IWorldRenderPass::PfNone,
					false
				);

				T_ASSERT(!renderContext->havePendingDraws());
				wc.build(worldRenderView, velocityPass, m_rootEntity);
				wc.flush(worldRenderView, velocityPass);
				renderContext->merge(render::RpAll);
			}
		);
	}
}

void WorldRendererDeferred::buildAmbientOcclusion(const WorldRenderView& worldRenderView)
{
	m_renderGraph->addPass(
		L"Ambient occlusion",
		[&](render::RenderPassBuilder& builder)
		{
			if (m_ambientOcclusion != nullptr)
				builder.addInput(s_handleGBuffer);

			render::Clear clear;
			clear.mask = render::CfColor;
			clear.colors[0] = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
			builder.setOutput(s_handleAmbientOcclusion, clear);
		},
		[=](render::RenderPassResources& resources, render::RenderContext* renderContext)
		{
			if (m_ambientOcclusion == nullptr)
				return;

			auto gbufferTargetSet = m_renderGraph->getRenderTarget(s_handleGBuffer);

			render::ImageProcessStep::Instance::RenderParams params;
			params.viewFrustum = worldRenderView.getViewFrustum();
			params.view = worldRenderView.getView();
			params.projection = worldRenderView.getProjection();
			params.deltaTime = 0.0f;

			m_ambientOcclusion->build(
				renderContext,
				nullptr,	// color
				gbufferTargetSet->getColorTexture(0),	// depth
				gbufferTargetSet->getColorTexture(1),	// normal
				nullptr,	// velocity
				nullptr,	// shadow mask
				params
			);
		}
	);
}

void WorldRendererDeferred::buildLights(const WorldRenderView& worldRenderView, int32_t frame, LightShaderData* lightShaderData, TileShaderData* tileShaderData)
{
	const UniformShadowProjection shadowProjection(1024);
	const auto shadowSettings = m_settings.shadowSettings[m_shadowsQuality];
	const bool shadowsEnable = (bool)(m_shadowsQuality != QuDisabled);

	Matrix44 view = worldRenderView.getView();
	Matrix44 viewInverse = view.inverse();
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
	if (lightCascadeIndex >= 0)
	{
		m_renderGraph->addPass(
			L"Shadow cascade",
			[&](render::RenderPassBuilder& builder)
			{
				render::Clear clear;
				clear.mask = render::CfDepth;
				clear.depth = 1.0f;
				builder.setOutput(s_handleShadowMapCascade, clear);
			},
			[=](render::RenderPassResources& resources, render::RenderContext* renderContext)
			{
				WorldContext wc(
					m_entityRenderers,
					renderContext,
					m_rootEntity
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

					WorldRenderPassDeferred shadowPass(
						s_techniqueShadow,
						sharedParams,
						shadowRenderView,
						IWorldRenderPass::PfNone,
						false
					);

					T_ASSERT(!renderContext->havePendingDraws());
					wc.build(shadowRenderView, shadowPass, m_rootEntity);
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
	}

	if (!lightAtlasIndices.empty())
	{
		int32_t atlasIndex = 0;
		for (int32_t lightAtlasIndex : lightAtlasIndices)
		{
			m_renderGraph->addPass(
				L"Shadow atlas",
				[&](render::RenderPassBuilder& builder)
				{
					render::Clear clear;
					clear.mask = render::CfDepth;
					clear.depth = 1.0f;
					builder.setOutput(s_handleShadowMapAtlas, clear);
				},
				[=](render::RenderPassResources& resources, render::RenderContext* renderContext)
				{
					WorldContext wc(
						m_entityRenderers,
						renderContext,
						m_rootEntity
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

					WorldRenderPassDeferred shadowPass(
						s_techniqueShadow,
						sharedParams,
						shadowRenderView,
						IWorldRenderPass::PfNone,
						false
					);

					T_ASSERT(!renderContext->havePendingDraws());
					wc.build(shadowRenderView, shadowPass, m_rootEntity);
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
			++atlasIndex;
		}
	}

	// Update tile data.
	{
		const float dx = 1.0f / 16.0f;
		const float dy = 1.0f / 16.0f;

		Vector4 nh = viewFrustum.corners[1] - viewFrustum.corners[0];
		Vector4 nv = viewFrustum.corners[3] - viewFrustum.corners[0];
		Vector4 fh = viewFrustum.corners[5] - viewFrustum.corners[4];
		Vector4 fv = viewFrustum.corners[7] - viewFrustum.corners[4];

		Frustum tileFrustum;
		for (int32_t y = 0; y < 16; ++y)
		{
			float fy = float(y) * dy;
			for (int32_t x = 0; x < 16; ++x)
			{
				float fx = float(x) * dx;

				Vector4 corners[] =
				{
					// Near
					viewFrustum.corners[0] + nh * Scalar(fx) + nv * Scalar(fy),				// l t
					viewFrustum.corners[0] + nh * Scalar(fx + dx) + nv * Scalar(fy),		// r t
					viewFrustum.corners[0] + nh * Scalar(fx + dx) + nv * Scalar(fy + dy),	// r b
					viewFrustum.corners[0] + nh * Scalar(fx) + nv * Scalar(fy + dy),		// l b
					// Far
					viewFrustum.corners[4] + fh * Scalar(fx) + fv * Scalar(fy),				// l t
					viewFrustum.corners[4] + fh * Scalar(fx + dx) + fv * Scalar(fy),		// r t
					viewFrustum.corners[4] + fh * Scalar(fx + dx) + fv * Scalar(fy + dy),	// r b
					viewFrustum.corners[4] + fh * Scalar(fx) + fv * Scalar(fy + dy)			// l b
				};

				tileFrustum.buildFromCorners(corners);

				int32_t count = 0;
				for (uint32_t i = 0; i < m_lights.size(); ++i)
				{
					const Light& light = m_lights[i];

					if (light.type == LtDirectional)
					{
						tileShaderData[x + y * 16].lights[count++] = float(i);
					}
					else if (light.type == LtPoint)
					{
						Vector4 lvp = worldRenderView.getView() * light.position.xyz1();
						if (tileFrustum.inside(lvp, Scalar(light.range)) != Frustum::IrOutside)
							tileShaderData[x + y * 16].lights[count++] = float(i);
					}
					else if (light.type == LtSpot)
					{
						tileShaderData[x + y * 16].lights[count++] = float(i);
					}

					if (count >= 4)
						break;
				}
				tileShaderData[x + y * 16].lightCount[0] = float(count);
			}
		}
	}
}

void WorldRendererDeferred::buildShadowMask(const WorldRenderView& worldRenderView)
{
	const auto shadowSettings = m_settings.shadowSettings[m_shadowsQuality];
	const bool shadowsEnable = (bool)(m_shadowsQuality != QuDisabled);

	if (!shadowsEnable)
		return;

	m_renderGraph->addPass(
		L"Shadow mask",
		[&](render::RenderPassBuilder& builder)
		{
			builder.addInput(s_handleShadowMapCascade);

			render::Clear clear;
			clear.mask = render::CfColor;
			clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
			builder.setOutput(s_handleShadowMask, clear);
		},
		[=](render::RenderPassResources& resources, render::RenderContext* renderContext)
		{
			auto gbufferTargetSet = m_renderGraph->getRenderTarget(s_handleGBuffer);
			auto shadowCascadeTargetSet = m_renderGraph->getRenderTarget(s_handleShadowMapCascade);

			for (int32_t slice = 0; slice < m_shadowSettings.cascadingSlices; ++slice)
			{
				Scalar zn(max(m_slicePositions[slice], m_settings.viewNearZ));
				Scalar zf(min(m_slicePositions[slice + 1], m_shadowSettings.farZ));

				render::ImageProcessStep::Instance::RenderParams params;
				params.viewFrustum = worldRenderView.getViewFrustum();
				//params.viewToLight = shadowLightProjection * shadowLightView * viewInverse;
				params.projection = worldRenderView.getProjection();
				params.sliceCount = m_shadowSettings.cascadingSlices;
				params.sliceIndex = slice;
				params.sliceNearZ = zn;
				params.sliceFarZ = zf;
				params.shadowFarZ = m_shadowSettings.farZ;
				params.shadowMapBias = m_shadowSettings.bias + slice * m_shadowSettings.biasCoeff;
				params.deltaTime = 0.0f;

				m_shadowMaskProject->build(
					renderContext,
					shadowCascadeTargetSet->getDepthTexture(),	// color
					gbufferTargetSet->getColorTexture(0),	// depth
					gbufferTargetSet->getColorTexture(1),	// normal
					nullptr,	// velocity
					nullptr,	// shadow mask
					params
				);
			}
		}
	);
}

void WorldRendererDeferred::buildReflections(const WorldRenderView& worldRenderView)
{
	if (m_reflectionsQuality == QuDisabled)
		return;

	m_renderGraph->addPass(
		L"Reflections",
		[&](render::RenderPassBuilder& builder)
		{
			builder.addInput(s_handleGBuffer);

			if (m_reflectionsQuality >= QuHigh)
				builder.addInput(s_handleColorReadBack);

			render::Clear clear;
			clear.mask = render::CfColor;
			clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
			builder.setOutput(s_handleReflections, clear);
		},
		[=](render::RenderPassResources& resources, render::RenderContext* renderContext)
		{
			WorldContext wc(
				m_entityRenderers,
				renderContext,
				m_rootEntity
			);

			auto gbufferTargetSet = m_renderGraph->getRenderTarget(s_handleGBuffer);
			auto colorReadBack = m_renderGraph->getRenderTarget(s_handleColorReadBack);

			auto sharedParams = renderContext->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(renderContext);
			sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
			sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
			sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
			sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
			sharedParams->setTextureParameter(s_handleDepthMap, gbufferTargetSet->getColorTexture(0));
			sharedParams->setTextureParameter(s_handleNormalMap, gbufferTargetSet->getColorTexture(1));
			sharedParams->setTextureParameter(s_handleMiscMap, gbufferTargetSet->getColorTexture(2));
			sharedParams->setTextureParameter(s_handleColorMap, gbufferTargetSet->getColorTexture(3));
			sharedParams->endParameters(renderContext);

			WorldRenderPassDeferred reflectionsPass(
				s_techniqueReflectionWrite,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::PfNone,
				false
			);

			T_ASSERT(!renderContext->havePendingDraws());
			wc.build(worldRenderView, reflectionsPass, m_rootEntity);
			wc.flush(worldRenderView, reflectionsPass);
			renderContext->merge(render::RpAll);

			// Render screenspace reflections.
			if (m_reflectionsQuality >= QuHigh && colorReadBack != nullptr)
			{
				m_lightRenderer->renderReflections(
					wc.getRenderContext(),
					worldRenderView.getProjection(),
					worldRenderView.getView(),
					worldRenderView.getLastView(),
					colorReadBack->getColorTexture(0),	// \tbd using last frame copy without reprojection...
					gbufferTargetSet->getColorTexture(0),	// depth
					gbufferTargetSet->getColorTexture(1),	// normals
					gbufferTargetSet->getColorTexture(2)	// metalness, roughness and specular
				);
			}
		}
	);
}

void WorldRendererDeferred::buildVisual(const WorldRenderView& worldRenderView, int32_t frame)
{
	const bool shadowsEnable = (bool)(m_shadowsQuality != QuDisabled);
	int32_t lightCount = (int32_t)m_lights.size();

	m_renderGraph->addPass(
		L"Visual",
		[&](render::RenderPassBuilder& builder)
		{
			builder.addInput(s_handleGBuffer);
			builder.addInput(s_handleAmbientOcclusion);
			builder.addInput(s_handleReflections);

			if (shadowsEnable)
			{
				builder.addInput(s_handleShadowMask);
				builder.addInput(s_handleShadowMapAtlas);
			}

			render::Clear clear;
			clear.mask = render::CfColor;
			clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 1.0f);
			builder.setOutput(s_handleVisual[0], clear);
		},
		[=](render::RenderPassResources& resources, render::RenderContext* renderContext)
		{
			WorldContext wc(
				m_entityRenderers,
				renderContext,
				m_rootEntity
			);

			auto gbufferTargetSet = m_renderGraph->getRenderTarget(s_handleGBuffer);
			auto ambientOcclusionTargetSet = m_renderGraph->getRenderTarget(s_handleAmbientOcclusion);
			auto reflectionsTargetSet = m_renderGraph->getRenderTarget(s_handleReflections);
			auto shadowMaskTargetSet = m_renderGraph->getRenderTarget(s_handleShadowMask);
			auto shadowAtlasTargetSet = m_renderGraph->getRenderTarget(s_handleShadowMapAtlas);

			auto sharedParams = renderContext->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(renderContext);
			sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
			sharedParams->setFloatParameter(s_handleLightCount, (float)m_frames[frame].lightCount);
			sharedParams->setVectorParameter(s_handleFogDistanceAndDensity, m_fogDistanceAndDensity);
			sharedParams->setVectorParameter(s_handleFogColor, m_fogColor);
			sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
			sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
			sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
			sharedParams->setTextureParameter(s_handleDepthMap, gbufferTargetSet->getColorTexture(0));
			sharedParams->setTextureParameter(s_handleNormalMap, gbufferTargetSet->getColorTexture(1));
			sharedParams->setTextureParameter(s_handleMiscMap, gbufferTargetSet->getColorTexture(2));
			sharedParams->setTextureParameter(s_handleColorMap, gbufferTargetSet->getColorTexture(3));
			sharedParams->setTextureParameter(s_handleOcclusionMap, ambientOcclusionTargetSet->getColorTexture(0));
			sharedParams->setStructBufferParameter(s_handleLightSBuffer, m_frames[frame].lightSBuffer);
			sharedParams->setStructBufferParameter(s_handleTileSBuffer, m_frames[frame].tileSBuffer);
			if (m_irradianceGrid)
			{
				const auto size = m_irradianceGrid->getSize();
				sharedParams->setVectorParameter(s_handleIrradianceGridSize, Vector4((float)size[0], (float)size[1], (float)size[2], 0.0f));
				sharedParams->setVectorParameter(s_handleIrradianceGridBoundsMin, m_irradianceGrid->getBoundingBox().mn);
				sharedParams->setVectorParameter(s_handleIrradianceGridBoundsMax, m_irradianceGrid->getBoundingBox().mx);
				sharedParams->setStructBufferParameter(s_handleIrradianceGridSBuffer, m_irradianceGrid->getBuffer());
			}
			sharedParams->endParameters(renderContext);

			// Irradiance
			WorldRenderPassDeferred irradiancePass(
				s_techniqueIrradianceWrite,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::PfNone,
				(bool)m_irradianceGrid
			);

			T_ASSERT(!renderContext->havePendingDraws());
			wc.build(worldRenderView, irradiancePass, m_rootEntity);
			wc.flush(worldRenderView, irradiancePass);
			renderContext->merge(render::RpAll);

			// Analytical lights; resolve with gbuffer.
			m_lightRenderer->renderLights(
				wc.getRenderContext(),
				worldRenderView.getTime(),
				m_frames[frame].lightCount,
				worldRenderView.getProjection(),
				worldRenderView.getView(),
				m_frames[frame].lightSBuffer,
				m_frames[frame].tileSBuffer,
				m_irradianceGrid,
				gbufferTargetSet->getColorTexture(0),	// depth
				gbufferTargetSet->getColorTexture(1),	// normals
				gbufferTargetSet->getColorTexture(2),	// metalness/roughness
				gbufferTargetSet->getColorTexture(3),	// surface color
				ambientOcclusionTargetSet->getColorTexture(0),	// ambient occlusion
				shadowMaskTargetSet != nullptr ? shadowMaskTargetSet->getColorTexture(0) : nullptr,		// shadow mask
				shadowAtlasTargetSet != nullptr ? shadowAtlasTargetSet->getDepthTexture() : nullptr,	// shadow map atlas,
				reflectionsTargetSet != nullptr ? reflectionsTargetSet->getColorTexture(0) : nullptr	// reflection map
			);

			// Fog
			if (dot4(m_fogDistanceAndDensity, Vector4(0.0f, 0.0f, 1.0f, 1.0f)) > FUZZY_EPSILON)
			{
				m_lightRenderer->renderFog(
					wc.getRenderContext(),
					worldRenderView.getProjection(),
					worldRenderView.getView(),
					m_fogDistanceAndDensity,
					m_fogColor,
					gbufferTargetSet->getColorTexture(0),
					gbufferTargetSet->getColorTexture(1),
					gbufferTargetSet->getColorTexture(2),
					gbufferTargetSet->getColorTexture(3)
				);
			}

			// Forward visuals; not included in GBuffer.
			WorldRenderPassDeferred deferredColorPass(
				s_techniqueDeferredColor,
				sharedParams,
				worldRenderView,
				IWorldRenderPass::PfLast,
				m_settings.fog,
				gbufferTargetSet->getColorTexture(0) != nullptr
			);

			T_ASSERT(!renderContext->havePendingDraws());
			wc.build(worldRenderView, deferredColorPass, m_rootEntity);
			wc.flush(worldRenderView, deferredColorPass);
			renderContext->merge(render::RpAll);
		}
	);
}

void WorldRendererDeferred::buildCopyFrame(const WorldRenderView& worldRenderView)
{
	m_renderGraph->addPass(
		L"Copy Frame",
		[&](render::RenderPassBuilder& builder)
		{
			builder.addInput(s_handleVisual[0]);
			builder.setOutput(s_handleColorReadBack);
		},
		[=](render::RenderPassResources& resources, render::RenderContext* renderContext)
		{
			auto visualTargetSet = resources.getInput(s_handleVisual[0]);

			render::ImageProcessStep::Instance::RenderParams params;
			params.viewFrustum = worldRenderView.getViewFrustum();
			params.projection = worldRenderView.getProjection();
			params.deltaTime = 0.0f;

			m_colorTargetCopy->build(
				renderContext,
				visualTargetSet->getColorTexture(0),	// color
				nullptr,	// depth
				nullptr,	// normal
				nullptr,	// velocity
				nullptr,	// shadow mask
				params
			);
		}
	);
}

void WorldRendererDeferred::buildEndFrame(const WorldRenderView& worldRenderView)
{
	render::ImageProcessStep::Instance::RenderParams params;
	params.viewFrustum = worldRenderView.getViewFrustum();
	params.viewToLight = Matrix44::identity(); //f.viewToLightSpace;
	params.view = worldRenderView.getView();
	params.projection = worldRenderView.getProjection();
	params.deltaTime = 1.0f / 60.0f; // deltaTime;

	StaticVector< render::ImageProcess*, 5 > processes;
	if (m_motionBlurImageProcess)
		processes.push_back(m_motionBlurImageProcess);
	if (m_toneMapImageProcess)
		processes.push_back(m_toneMapImageProcess);
	if (m_visualImageProcess)
		processes.push_back(m_visualImageProcess);
	if (m_gammaCorrectionImageProcess)
		processes.push_back(m_gammaCorrectionImageProcess);
	if (m_antiAlias)
		processes.push_back(m_antiAlias);

	for (size_t i = 0; i < processes.size(); ++i)
	{
		auto process = processes[i];
		T_ASSERT(process != nullptr);

		bool haveNext = bool((i + 1) < processes.size());
		m_renderGraph->addPass(
			L"Process",
			[&](render::RenderPassBuilder& builder)
			{
				if (haveNext)
					builder.setOutput(s_handleVisual[i + 1]);

				builder.addInput(s_handleGBuffer);
				builder.addInput(s_handleVisual[i]);

				if (m_motionBlurQuality != QuDisabled)
					builder.addInput(s_handleVelocity);
			},
			[=](render::RenderPassResources& resources, render::RenderContext* renderContext)
			{
				auto gbufferTargetSet = resources.getInput(s_handleGBuffer);
				auto visualTargetSet = resources.getInput(s_handleVisual[i]);
				auto velocityTargetSet = resources.getInput(s_handleVelocity);

				render::ImageProcessStep::Instance::RenderParams params;
				params.viewFrustum = worldRenderView.getViewFrustum();
				params.view = worldRenderView.getView();
				params.projection = worldRenderView.getProjection();
				params.deltaTime = 1.0f / 60.0f; // \tbd deltaTime;

				process->build(
					renderContext,
					visualTargetSet->getColorTexture(0),	// color
					gbufferTargetSet->getColorTexture(0),	// depth
					gbufferTargetSet->getColorTexture(1),	// normal
					velocityTargetSet ? velocityTargetSet->getColorTexture(0) : nullptr,	// velocity
					nullptr,	// shadow mask
					params
				);
			}
		);
	}
}

	}
}
