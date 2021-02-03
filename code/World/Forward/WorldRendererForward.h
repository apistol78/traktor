#pragma once

#include "Core/Containers/AlignedVector.h"
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

class Job;

	namespace render
	{

class ImageGraph;
class ISimpleTexture;
class ScreenRenderer;
class StructBuffer;

	}

	namespace world
	{

class IrradianceGrid;
class LightComponent;
class Packer;
class ProbeComponent;
class WorldEntityRenderers;
struct LightShaderData;
struct TileShaderData;

/*! World renderer implementation.
 * \ingroup World
 *
 * Simple and naive implementation supporting
 * only a single shadowing directional light
 * and max 2 overlapping lights.
 *
 * Operation
 * 1. Render global shadow map if directional light 0 enabled.
 * 2. Render depth pass if enabled (either explicitly or shadows enabled).
 * 3. Render screen-space shadow mask.
 * 4. Render visuals.
 *
 * Techniques used
 * "Default" - Visual, final, color output.
 * "Depth"   - Depth pass; write view distance into depth buffer.
 * "Shadow"  - Shadow map pass; write distance from light into shadow map.
 */
class T_DLLCLASS WorldRendererForward : public IWorldRenderer
{
	T_RTTI_CLASS;

public:
	virtual bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const WorldCreateDesc& desc
	) override final;

	virtual void destroy() override final;

	virtual void setup(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId
	) override final;

private:
	friend class TilesOverlay;

	struct Frame
	{
		Ref< render::StructBuffer > lightSBuffer;
		Ref< render::StructBuffer > lightIndexSBuffer;
		Ref< render::StructBuffer > tileSBuffer;
		AlignedVector< const LightComponent* > lights;
		AlignedVector< const ProbeComponent* > probes;
		Ref< Packer > shadowAtlasPacker;
		Ref< Job > tileJob;
	};

	WorldRenderSettings m_settings;
	Quality m_toneMapQuality = Quality::Disabled;
	Quality m_motionBlurQuality = Quality::Disabled;
	Quality m_shadowsQuality = Quality::Disabled;
	Quality m_reflectionsQuality = Quality::Disabled;
	Quality m_ambientOcclusionQuality = Quality::Disabled;
	Quality m_antiAliasQuality = Quality::Disabled;

	float m_gamma = 1.0f;

	Ref< render::IRenderTargetSet > m_sharedDepthStencil;
	Ref< render::ScreenRenderer > m_screenRenderer;
	
	Ref< render::ISimpleTexture > m_blackTexture;
	Ref< render::ISimpleTexture > m_whiteTexture;

	resource::Proxy< render::ImageGraph > m_velocityPrime;
	resource::Proxy< render::ImageGraph > m_ambientOcclusion;
	resource::Proxy< render::ImageGraph > m_antiAlias;
	resource::Proxy< render::ImageGraph > m_visual;
	resource::Proxy< render::ImageGraph > m_gammaCorrection;
	resource::Proxy< render::ImageGraph > m_toneMap;
	resource::Proxy< render::ImageGraph > m_screenReflections;

	resource::Proxy< IrradianceGrid > m_irradianceGrid;

	Ref< WorldEntityRenderers > m_entityRenderers;

	AlignedVector< Frame > m_frames;

	float m_slicePositions[MaxSliceCount + 1];
	int32_t m_count = 0;

	void setupTileDataPass(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId,
		int32_t frame
	);

	render::handle_t setupGBufferPass(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId
	) const;

	render::handle_t setupVelocityPass(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId,
		render::handle_t gbufferTargetSetId
	) const;

	render::handle_t setupAmbientOcclusionPass(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId,
		render::handle_t gbufferTargetSetId
	) const;

	render::handle_t setupReflectionsPass(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId,
		render::handle_t gbufferTargetSetId,
		render::handle_t visualReadTargetSetId
	) const;

	void setupLightPass(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId,
		int32_t frame,
		render::handle_t& outShadowMapAtlasTargetSetId
	) const;

	void setupVisualPass(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t visualWriteTargetSetId,
		render::handle_t gbufferTargetSetId,
		render::handle_t ambientOcclusionTargetSetId,
		render::handle_t reflectionsTargetSetId,
		render::handle_t shadowMapAtlasTargetSetId,
		int32_t frame
	);

	void setupProcessPass(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId,
		render::handle_t gbufferTargetSetId,
		render::handle_t velocityTargetSetId,
		render::handle_t visualWriteTargetSetId,
		render::handle_t visualReadTargetSetId
	) const;
};

	}
}
