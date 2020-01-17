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

class ImageProcess;
class RenderContext;
class RenderGraph;
class StructBuffer;

	}

	namespace world
	{

class GroupEntity;
class WorldEntityRenderers;

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

	virtual void build(const WorldRenderView& worldRenderView, int32_t frame) override final;

	virtual void render(render::IRenderView* renderView, int32_t frame) override final;

	virtual render::ImageProcess* getVisualImageProcess() override final;

	virtual void getDebugTargets(std::vector< render::DebugTarget >& outTargets) const override final;

private:
	struct Frame
	{
		Ref< render::RenderContext > renderContext;
		Ref< render::StructBuffer > lightSBuffer;
	};

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

	WorldRenderSettings m_settings;
	Quality m_toneMapQuality;
	Quality m_shadowsQuality;
	Quality m_ambientOcclusionQuality;
	Quality m_antiAliasQuality;

	Ref< render::RenderGraph > m_renderGraph;
	
	Ref< render::ImageProcess > m_ambientOcclusion;
	Ref< render::ImageProcess > m_antiAlias;
	Ref< render::ImageProcess > m_visualImageProcess;
	Ref< render::ImageProcess > m_gammaCorrectionImageProcess;
	Ref< render::ImageProcess > m_toneMapImageProcess;

	Ref< WorldEntityRenderers > m_entityRenderers;
	Ref< GroupEntity > m_rootEntity;
	AlignedVector< Frame > m_frames;
	AlignedVector< Light > m_lights;

	LightShaderData* m_lightShaderData;

	float m_slicePositions[MaxSliceCount + 1];
	uint32_t m_count;

	void buildGBuffer(const WorldRenderView& worldRenderView);

	void buildAmbientOcclusion(const WorldRenderView& worldRenderView);

	void buildLights(const WorldRenderView& worldRenderView, int32_t frame);

	void buildVisual(const WorldRenderView& worldRenderView, int32_t frame);

	void buildProcess(const WorldRenderView& worldRenderView);
};

	}
}
