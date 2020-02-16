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
	namespace render
	{

class ImageGraph;
class ScreenRenderer;
class StructBuffer;

	}

	namespace world
	{

class GroupEntity;
class WorldEntityRenderers;

struct LightShaderData;

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
	WorldRendererForward();

	virtual bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const WorldCreateDesc& desc
	) override final;

	virtual void destroy() override final;

	virtual void attach(Entity* entity) override final;

	virtual void setup(const WorldRenderView& worldRenderView, render::RenderGraph& renderGraph) override final;

private:
	struct Frame
	{
		Ref< render::StructBuffer > lightSBuffer;
		void* lightSBufferMemory;
	};

	WorldRenderSettings m_settings;
	Quality m_toneMapQuality;
	Quality m_shadowsQuality;
	Quality m_ambientOcclusionQuality;
	Quality m_antiAliasQuality;

	Ref< render::IRenderTargetSet > m_sharedDepthStencil;
	Ref< render::ScreenRenderer > m_screenRenderer;
	
	resource::Proxy< render::ImageGraph > m_ambientOcclusion;
	resource::Proxy< render::ImageGraph > m_antiAlias;
	resource::Proxy< render::ImageGraph > m_visual;
	resource::Proxy< render::ImageGraph > m_gammaCorrection;
	resource::Proxy< render::ImageGraph > m_toneMap;

	Ref< WorldEntityRenderers > m_entityRenderers;
	Ref< GroupEntity > m_rootEntity;
	AlignedVector< Frame > m_frames;
	AlignedVector< Light > m_lights;

	float m_slicePositions[MaxSliceCount + 1];
	int32_t m_count;

	render::handle_t setupGBufferPass(
		const WorldRenderView& worldRenderView,
		render::RenderGraph& renderGraph
	) const;

	render::handle_t setupAmbientOcclusionPass(
		const WorldRenderView& worldRenderView,
		render::RenderGraph& renderGraph,
		render::handle_t gbufferTargetSetId
	) const;

	void setupLightPass(
		const WorldRenderView& worldRenderView,
		render::RenderGraph& renderGraph,
		int32_t frame,
		LightShaderData* lightShaderData,
		render::handle_t& outShadowMapCascadeTargetSetId,
		render::handle_t& outShadowMapAtlasTargetSetId
	) const;

	render::handle_t setupVisualPass(
		const WorldRenderView& worldRenderView,
		render::RenderGraph& renderGraph,
		render::handle_t gbufferTargetSetId,
		render::handle_t ambientOcclusionTargetSetId,
		render::handle_t shadowMapCascadeTargetSetId,
		render::handle_t shadowMapAtlasTargetSetId,
		int32_t frame
	) const;

	void setupProcessPass(
		const WorldRenderView& worldRenderView,
		render::RenderGraph& renderGraph,
		render::handle_t gbufferTargetSetId,
		render::handle_t visualTargetSetId
	) const;
};

	}
}
