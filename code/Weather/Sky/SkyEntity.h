#ifndef traktor_weather_SkyEntity_H
#define traktor_weather_SkyEntity_H

#include "Render/Shader.h"
#include "Resource/Proxy.h"
#include "World/Entity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class IWorldRenderPass;
class WorldRenderView;

	}

	namespace render
	{

class RenderContext;
class VertexBuffer;
class IndexBuffer;

	}

	namespace weather
	{

class T_DLLCLASS SkyEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	SkyEntity(
		render::VertexBuffer* vertexBuffer,
		render::IndexBuffer* indexBuffer,
		const render::Primitives& primitives,
		const resource::Proxy< render::Shader >& shader,
		const Vector4& sunDirection,
		float offset
	);

	void setSunDirection(const Vector4& sunDirection);

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
	Vector4 m_sunDirection;
	float m_offset;
};

	}
}

#endif	// traktor_weather_SkyEntity_H
