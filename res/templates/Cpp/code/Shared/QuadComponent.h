#ifndef QuadComponent_H
#define QuadComponent_H

#include <Core/Math/Color4f.h>
#include <Render/Shader.h>
#include <Render/VertexBuffer.h>
#include <Render/Context/RenderContext.h>
#include <Resource/Proxy.h>
#include <World/IEntityComponent.h>
#include <World/WorldContext.h>

class QuadComponent : public traktor::world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	QuadComponent(
		const traktor::resource::Proxy< traktor::render::Shader >& shader,
		traktor::render::VertexBuffer* vertexBuffer
	);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setTransform(const traktor::Transform& transform) T_OVERRIDE T_FINAL;

	virtual traktor::Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual void update(const traktor::world::UpdateParams& update) T_OVERRIDE T_FINAL;

	void setColor(const traktor::Color4f& color);

	void render(
		traktor::render::RenderContext* renderContext,
		traktor::world::WorldRenderView& worldRenderView,
		traktor::world::IWorldRenderPass& worldRenderPass
	);

private:
	traktor::resource::Proxy< traktor::render::Shader > m_shader;
	traktor::Ref< traktor::render::VertexBuffer > m_vertexBuffer;
	traktor::Color4f m_color;
};

#endif	// QuadComponent_H
