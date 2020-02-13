#pragma once

#include "Core/RefArray.h"
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
class VertexBuffer;

	}

	namespace world
	{

class DecalComponent;

/*! Decal entity renderer.
 * \ingroup World
 */
class T_DLLCLASS DecalRenderer : public IEntityRenderer
{
	T_RTTI_CLASS;

public:
	DecalRenderer(render::IRenderSystem* renderSystem);

	virtual const TypeInfoSet getRenderableTypes() const override final;

	virtual void gather(
		const WorldGatherContext& context,
		const Object* renderable,
		AlignedVector< Light >& outLights
	) override final;

	virtual void build(
		const WorldBuildContext& context,
		const WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass,
		Object* renderable
	) override final;

	virtual void flush(
		const WorldBuildContext& context,
		const WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass
	) override final;

	virtual void flush(const WorldBuildContext& context) override final;

private:
	Ref< render::VertexBuffer > m_vertexBuffer;
	Ref< render::IndexBuffer > m_indexBuffer;
	RefArray< DecalComponent > m_decalComponents;
};

	}
}

