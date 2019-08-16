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
class Shader;
class VertexBuffer;

	}

	namespace resource
	{
	
class IResourceManager;

	}

	namespace world
	{

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
		render::IRenderSystem* renderSystem
	);

	virtual const TypeInfoSet getRenderableTypes() const override final;

	virtual void render(
		WorldContext& worldContext,
		WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass,
		Object* renderable
	) override final;

	virtual void flush(
		WorldContext& worldContext,
		WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass
	) override final;

private:
	resource::Proxy< render::Shader > m_probeShader;
	Ref< render::VertexBuffer > m_vertexBuffer;
	Ref< render::IndexBuffer > m_indexBuffer;
	RefArray< ProbeComponent > m_probeComponents;
};

	}
}
