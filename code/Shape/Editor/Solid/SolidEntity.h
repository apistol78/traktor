#pragma once

#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "World/Entity/GroupEntity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
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

    namespace world
    {

class IWorldRenderPass;
class WorldContext;
class WorldRenderView;

    }

    namespace shape
    {

class T_DLLCLASS SolidEntity : public world::GroupEntity
{
    T_RTTI_CLASS;

public:
    SolidEntity(
		render::IRenderSystem* renderSystem,
		const resource::Proxy< render::Shader >& shader,
		const Transform& transform
	);

    virtual void update(const world::UpdateParams& update) override final;

	void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass
	);

private:
	Ref< render::IRenderSystem > m_renderSystem;
	resource::Proxy< render::Shader > m_shader;
	Ref< render::VertexBuffer > m_vertexBuffer;
	Ref< render::IndexBuffer > m_indexBuffer;
	render::Primitives m_primitives;
};

    }
}