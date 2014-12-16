#ifndef traktor_terrain_RiverEntity_H
#define traktor_terrain_RiverEntity_H

#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "World/Entity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
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
class RenderContext;
class Shader;
class VertexBuffer;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace world
	{

class IWorldRenderPass;
class WorldRenderView;

	}

	namespace terrain
	{

class RiverEntityData;

/*! \brief River entity.
 * \ingroup Terrain
 */
class T_DLLCLASS RiverEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	RiverEntity();

	bool create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const RiverEntityData& data);

	void render(
		render::RenderContext* renderContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass
	);

	virtual Aabb3 getBoundingBox() const;

	virtual void update(const world::UpdateParams& update);

private:
	Ref< render::VertexBuffer > m_vertexBuffer;
	Ref< render::IndexBuffer > m_indexBuffer;
	render::Primitives m_primitives;
	resource::Proxy< render::Shader > m_shader;
};

	}
}

#endif	// traktor_terrain_RiverEntity_H
