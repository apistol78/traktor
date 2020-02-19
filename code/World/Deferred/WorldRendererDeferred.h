#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Misc/AutoPtr.h"
#include "Resource/Proxy.h"
#include "World/IWorldRenderer.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldRenderView.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ImageGraph;
class IRenderTargetSet;
class ScreenRenderer;
class StructBuffer;

	}

	namespace world
	{

class GroupEntity;
class IrradianceGrid;
class LightRendererDeferred;
class WorldEntityRenderers;

struct LightShaderData;
struct TileShaderData;

/*! World renderer, using deferred rendering method.
 * \ingroup World
 *
 * Advanced deferred rendering path.
 *
 * Operation
 * \TBD
 *
 * Techniques used
 * \TBD
 */
class T_DLLCLASS WorldRendererDeferred : public IWorldRenderer
{
	T_RTTI_CLASS;

public:
	WorldRendererDeferred();

	virtual bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const WorldCreateDesc& desc
	) override final;

	virtual void destroy() override final;

	virtual void attach(Entity* entity) override final;

	virtual void setup(
		const WorldRenderView& worldRenderView,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId
	) override final;

private:
	struct Frame
	{
		Ref< render::StructBuffer > lightSBuffer;
		void* lightSBufferMemory;
		Ref< render::StructBuffer > tileSBuffer;
		void* tileSBufferMemory;
	};

	WorldRenderSettings m_settings;
	WorldRenderSettings::ShadowSettings m_shadowSettings;

	Quality m_toneMapQuality;
	Quality m_motionBlurQuality;
	Quality m_shadowsQuality;
	Quality m_reflectionsQuality;
	Quality m_ambientOcclusionQuality;
	Quality m_antiAliasQuality;

	Ref< render::IRenderTargetSet > m_sharedDepthStencil;

	Ref< render::ScreenRenderer > m_screenRenderer;
	Ref< LightRendererDeferred > m_lightRenderer;

	render::Handle m_handleShadowMapCascade;
	render::Handle m_handleShadowMapAtlas;
	render::Handle m_handleVisual[7];

	resource::Proxy< render::ImageGraph > m_ambientOcclusion;
	resource::Proxy< render::ImageGraph > m_antiAlias;
	resource::Proxy< render::ImageGraph > m_visual;
	resource::Proxy< render::ImageGraph > m_gammaCorrection;
	resource::Proxy< render::ImageGraph > m_motionBlurPrime;
	resource::Proxy< render::ImageGraph > m_motionBlur;
	resource::Proxy< render::ImageGraph > m_toneMap;
	resource::Proxy< render::ImageGraph > m_shadowMaskProject;

	resource::Proxy< IrradianceGrid > m_irradianceGrid;
	
	Ref< WorldEntityRenderers > m_entityRenderers;
	Ref< GroupEntity > m_rootEntity;
	AlignedVector< Frame > m_frames;
	AlignedVector< Light > m_lights;

	float m_slicePositions[MaxSliceCount + 1];
	Vector4 m_fogDistanceAndDensity;
	Vector4 m_fogColor;

	int32_t m_count;

	render::handle_t setupGBufferPass(
		const WorldRenderView& worldRenderView,
		render::RenderGraph& renderGraph
	) const;

	render::handle_t setupVelocityPass(
		const WorldRenderView& worldRenderView,
		render::RenderGraph& renderGraph,
		render::handle_t gbufferTargetSetId
	) const;

	render::handle_t setupAmbientOcclusionPass(
		const WorldRenderView& worldRenderView,
		render::RenderGraph& renderGraph,
		render::handle_t gbufferTargetSetId
	) const;

	render::handle_t setupCascadeShadowMapPass(
		const WorldRenderView& worldRenderView,
		render::RenderGraph& renderGraph,
		int32_t lightCascadeIndex,
		LightShaderData* lightShaderData
	) const;

	render::handle_t setupAtlasShadowMapPass(
		const WorldRenderView& worldRenderView,
		render::RenderGraph& renderGraph,
		const StaticVector< int32_t, 16 >& lightAtlasIndices,
		LightShaderData* lightShaderData
	) const;

	void setupTileDataPass(
		const WorldRenderView& worldRenderView,
		render::RenderGraph& renderGraph,
		TileShaderData* tileShaderData
	) const;

	render::handle_t setupShadowMaskPass(
		const WorldRenderView& worldRenderView,
		render::RenderGraph& renderGraph,
		render::handle_t gbufferTargetSetId,
		render::handle_t shadowMapCascadeTargetSetId,
		int32_t lightCascadeIndex
	) const;

	render::handle_t setupReflectionsPass(
		const WorldRenderView& worldRenderView,
		render::RenderGraph& renderGraph,
		render::handle_t gbufferTargetSetId,
		render::handle_t visualTargetSetId
	) const;

	render::handle_t setupVisualPass(
		const WorldRenderView& worldRenderView,
		render::RenderGraph& renderGraph,
		render::handle_t gbufferTargetSetId,
		render::handle_t ambientOcclusionTargetSetId,
		render::handle_t reflectionsTargetSetId,
		render::handle_t shadowMaskTargetSetId,
		render::handle_t shadowMapAtlasTargetSetId,
		int32_t frame
	) const;

	void setupProcessPass(
		const WorldRenderView& worldRenderView,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId,
		render::handle_t gbufferTargetSetId,
		render::handle_t velocityTargetSetId,
		render::handle_t visualTargetSetId
	) const;
};

	}
}
