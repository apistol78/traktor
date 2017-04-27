/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_terrain_RiverComponent_H
#define traktor_terrain_RiverComponent_H

#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "World/IEntityComponent.h"

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

class RiverComponentData;

/*! \brief River component.
 * \ingroup Terrain
 */
class T_DLLCLASS RiverComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	bool create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const RiverComponentData& data);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setOwner(world::Entity* owner) T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual void update(const world::UpdateParams& update) T_OVERRIDE T_FINAL;

	void render(
		render::RenderContext* renderContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass
	);

private:
	Ref< render::VertexBuffer > m_vertexBuffer;
	Ref< render::IndexBuffer > m_indexBuffer;
	render::Primitives m_primitives;
	resource::Proxy< render::Shader > m_shader;
};

	}
}

#endif	// traktor_terrain_RiverComponent_H
