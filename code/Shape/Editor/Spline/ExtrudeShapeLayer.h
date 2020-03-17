#pragma once

#include "Core/Ref.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "Shape/Editor/Spline/SplineLayerComponent.h"

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
class WorldBuildContext;
class WorldRenderView;

	}

	namespace shape
	{

/*!
 * \ingroup Shape
 */
class T_DLLCLASS ExtrudeShapeLayer : public SplineLayerComponent
{
	T_RTTI_CLASS;

public:
	ExtrudeShapeLayer(
		render::IRenderSystem* renderSystem,
		const resource::Proxy< render::Shader >& shader,
		bool automaticOrientation,
		float detail
	);

	virtual void destroy() override final;

	virtual void setOwner(world::ComponentEntity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	virtual void pathChanged(const TransformPath& path) override final;

	void build(
		const world::WorldBuildContext& context,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass
	);

private:
	Ref< render::IRenderSystem > m_renderSystem;
	resource::Proxy< render::Shader > m_shader;
	bool m_automaticOrientation;
	float m_detail;

	Ref< render::VertexBuffer > m_vertexBuffer;
	Ref< render::IndexBuffer > m_indexBuffer;
	render::Primitives m_primitives;
};

	}
}
