#pragma once

#include "Core/Ref.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "Shape/Spline/ISplineLayer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EXPORT)
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

	namespace shape
	{

class SplineEntity;

/*! \brief
 * \ingroup Shape
 */
class T_DLLCLASS ExtrudeShapeLayer : public ISplineLayer
{
	T_RTTI_CLASS;

public:
	ExtrudeShapeLayer(
		SplineEntity* owner,
		render::IRenderSystem* renderSystem,
		const resource::Proxy< render::Shader >& shader,
		bool automaticOrientation,
		float detail
	);

	virtual void update(const world::UpdateParams& update) override final;

	virtual void pathChanged() override final;

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass
	) override final;

private:
	SplineEntity* m_owner;
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
