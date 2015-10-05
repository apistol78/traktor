#ifndef traktor_world_DecalEntityRenderer_H
#define traktor_world_DecalEntityRenderer_H

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

class DecalEntity;

/*! \brief Decal entity renderer.
 * \ingroup World
 */
class T_DLLCLASS DecalEntityRenderer : public IEntityRenderer
{
	T_RTTI_CLASS;

public:
	DecalEntityRenderer(render::IRenderSystem* renderSystem);

	virtual const TypeInfoSet getEntityTypes() const;

	virtual void precull(
		WorldContext& worldContext,
		WorldRenderView& worldRenderView,
		Entity* entity
	) T_OVERRIDE T_FINAL;

	virtual void render(
		WorldContext& worldContext,
		WorldRenderView& worldRenderView,
		IWorldRenderPass& worldRenderPass,
		Entity* entity
	) T_OVERRIDE T_FINAL;

	virtual void flush(
		WorldContext& worldContext,
		WorldRenderView& worldRenderView,
		IWorldRenderPass& worldRenderPass
	) T_OVERRIDE T_FINAL;

private:
	Ref< render::VertexBuffer > m_vertexBuffer;
	Ref< render::IndexBuffer > m_indexBuffer;
	RefArray< DecalEntity > m_decalEntities;
};

	}
}

#endif	// traktor_world_DecalEntityRenderer_H
