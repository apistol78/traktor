#pragma once

#include "Core/RefArray.h"
#include "Resource/Proxy.h"
#include "World/IEntityRenderer.h"

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

class IndexBuffer;
class IRenderSystem;
class IRenderTargetSet;
class ScreenRenderer;
class Shader;
class VertexBuffer;

	}

	namespace resource
	{
	
class IResourceManager;

	}

	namespace world
	{

class IWorldRenderer;
class ProbeComponent;

/*! Probe entity renderer.
 * \ingroup World
 */
class T_DLLCLASS ProbeRenderer : public IEntityRenderer
{
	T_RTTI_CLASS;

public:
	ProbeRenderer(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const TypeInfo& worldRendererType
	);

	virtual const TypeInfoSet getRenderableTypes() const override final;

	virtual void gather(
		const WorldGatherContext& context,
		const Object* renderable,
		AlignedVector< const LightComponent* >& outLights,
		AlignedVector< const ProbeComponent* >& outProbes
	) override final;

	virtual void setup(
		const WorldSetupContext& context,
		const WorldRenderView& worldRenderView,
		Object* renderable
	) override final;

	virtual void setup(
		const WorldSetupContext& context
	) override final;

	virtual void build(
		const WorldBuildContext& context,
		const WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass,
		Object* renderable
	) override final;

	virtual void build(
		const WorldBuildContext& context,
		const WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass
	) override final;

private:
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	const TypeInfo& m_worldRendererType;
	Ref< IWorldRenderer > m_worldRenderer;
	resource::Proxy< render::Shader > m_probeShader;
	resource::Proxy< render::Shader > m_filterShader;
	Ref< render::VertexBuffer > m_vertexBuffer;
	Ref< render::IndexBuffer > m_indexBuffer;
	Ref< render::ScreenRenderer > m_screenRenderer;
	Ref< render::IRenderTargetSet > m_depthTargetSet;
	RefArray< ProbeComponent > m_captureQueue;	//!< Probes queued for capture.
	Ref< ProbeComponent > m_capture;			//!< Probe being captured.
	int32_t m_captureState;
};

	}
}
