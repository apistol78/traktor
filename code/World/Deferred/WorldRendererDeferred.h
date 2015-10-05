#ifndef traktor_world_WorldRendererDeferred_H
#define traktor_world_WorldRendererDeferred_H

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
class ISimpleTexture;
class RenderContext;
class RenderTargetSet;

	}

	namespace world
	{

class IWorldShadowProjection;
class LightRendererDeferred;
class WorldContext;
class WorldCullingSwRaster;

/*! \brief World renderer deferred implementation.
 * \ingroup World
 */
class T_DLLCLASS WorldRendererDeferred : public IWorldRenderer
{
	T_RTTI_CLASS;

public:
	WorldRendererDeferred();

	virtual bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::IRenderView* renderView,
		const WorldCreateDesc& desc
	) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool beginBuild() T_OVERRIDE T_FINAL;

	virtual void build(Entity* entity) T_OVERRIDE T_FINAL;

	virtual void endBuild(WorldRenderView& worldRenderView, int frame) T_OVERRIDE T_FINAL;

	virtual bool beginRender(int frame, render::EyeType eye, const Color4f& clearColor) T_OVERRIDE T_FINAL;

	virtual void render(uint32_t flags, int frame, render::EyeType eye) T_OVERRIDE T_FINAL;

	virtual void endRender(int frame, render::EyeType eye, float deltaTime) T_OVERRIDE T_FINAL;

	virtual render::ImageProcess* getVisualImageProcess() T_OVERRIDE T_FINAL;

	virtual void getDebugTargets(std::vector< DebugTarget >& outTargets) const T_OVERRIDE T_FINAL;

private:
	struct Slice
	{
		Ref< WorldContext > shadow[MaxLightShadowCount];
		Matrix44 shadowLightView[MaxLightShadowCount];
		Matrix44 shadowLightProjection[MaxLightShadowCount];
		Matrix44 viewToLightSpace[MaxLightShadowCount];
	};

	struct Frame
	{
		Ref< WorldCullingSwRaster > culling;
		Slice slice[MaxSliceCount];
		Ref< WorldContext > gbuffer;
		Ref< WorldContext > visual;
		float time;
		float A;
		float B;
		Matrix44 projection;
		Matrix44 view;
		Frustum viewFrustum;
		Light lights[MaxLightCount];
		uint32_t lightCount;
		Vector4 godRayDirection;
		bool haveGBuffer;
		bool haveShadows[MaxLightCount];

		Frame()
		:	time(0.0f)
		,	A(0.0f)
		,	B(0.0f)
		,	lightCount(0)
		,	haveGBuffer(false)
		{
			for (uint32_t i = 0; i < MaxLightCount; ++i)
				haveShadows[i] = false;
		}
	};

	static render::handle_t ms_techniqueDeferredColor;
	static render::handle_t ms_techniqueDeferredGBufferWrite;
	static render::handle_t ms_techniqueShadow;
	static render::handle_t ms_handleTime;
	static render::handle_t ms_handleView;
	static render::handle_t ms_handleViewInverse;
	static render::handle_t ms_handleProjection;
	static render::handle_t ms_handleColorMap;
	static render::handle_t ms_handleDepthMap;
	static render::handle_t ms_handleLightMap;
	static render::handle_t ms_handleNormalMap;
	static render::handle_t ms_handleReflectionMap;
	static render::handle_t ms_handleFogDistanceAndDensity;
	static render::handle_t ms_handleFogColor;

	WorldRenderSettings m_settings;
	WorldRenderSettings::ShadowSettings m_shadowSettings;
	Quality m_shadowsQuality;
	Quality m_ambientOcclusionQuality;
	Quality m_antiAliasQuality;

	Ref< render::IRenderView > m_renderView;
	Ref< IWorldShadowProjection > m_shadowProjection0;
	Ref< IWorldShadowProjection > m_shadowProjection;
	Ref< render::RenderTargetSet > m_visualTargetSet;
	Ref< render::RenderTargetSet > m_intermediateTargetSet;
	Ref< render::RenderTargetSet > m_gbufferTargetSet;
	Ref< render::RenderTargetSet > m_colorTargetSet;
	Ref< render::RenderTargetSet > m_shadowTargetSet;
	Ref< render::RenderTargetSet > m_shadowMaskProjectTargetSet;
	Ref< render::RenderTargetSet > m_shadowMaskFilterTargetSet;
	Ref< render::RenderContext > m_globalContext;
	resource::Proxy< render::ITexture > m_reflectionMap;
	Ref< render::ImageProcess > m_shadowMaskProject;
	Ref< render::ImageProcess > m_shadowMaskFilter;
	Ref< render::ImageProcess > m_colorTargetCopy;
	Ref< render::ImageProcess > m_ambientOcclusion;
	Ref< render::ImageProcess > m_antiAlias;
	Ref< render::ImageProcess > m_visualImageProcess;
	Ref< render::ImageProcess > m_gammaCorrectionImageProcess;
	Ref< LightRendererDeferred > m_lightRenderer;
	RefArray< Entity > m_buildEntities;
	AlignedVector< Frame > m_frames;
	float m_slicePositions[MaxSliceCount + 1];
	uint32_t m_count;
	Vector4 m_fogDistanceAndDensity;
	Vector4 m_fogColor;


	void buildGBuffer(WorldRenderView& worldRenderView, int frame);

	void buildLightWithShadows(WorldRenderView& worldRenderView, int frame);

	void buildLightWithNoShadows(WorldRenderView& worldRenderView, int frame);

	void buildVisual(WorldRenderView& worldRenderView, int frame);
};

	}
}

#endif	// traktor_world_WorldRendererDeferred_H
