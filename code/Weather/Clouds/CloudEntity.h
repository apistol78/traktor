#ifndef traktor_weather_CloudEntity_H
#define traktor_weather_CloudEntity_H

#include "Resource/Proxy.h"
#include "World/Entity/SpatialEntity.h"
#include "Weather/Clouds/CloudParticleCluster.h"
#include "Weather/Clouds/CloudParticleData.h"
#include "Render/Shader.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class RenderSystem;
class RenderContext;
class PrimitiveRenderer;
class RenderTargetSet;
class VertexBuffer;
class IndexBuffer;

	}

	namespace world
	{

class WorldRenderView;

	}

	namespace weather
	{

class T_DLLCLASS CloudEntity : public world::SpatialEntity
{
	T_RTTI_CLASS(CloudEntity)

public:
	CloudEntity();

	bool create(
		render::RenderSystem* renderSystem,
		const resource::Proxy< render::Shader >& particleShader,
		const resource::Proxy< render::Shader >& impostorShader,
		uint32_t impostorTargetResolution,
		uint32_t distanceTargetResolution,
		const CloudParticleData& particleData
	);

	void render(render::RenderContext* renderContext, const world::WorldRenderView* worldRenderView, render::PrimitiveRenderer* primitiveRenderer);

	virtual void update(const world::EntityUpdate* update);

	virtual void setTransform(const Matrix44& transform);

	virtual bool getTransform(Matrix44& outTransform) const;

	virtual Aabb getBoundingBox() const;

private:
	resource::Proxy< render::Shader > m_particleShader;
	resource::Proxy< render::Shader > m_impostorShader;
	Ref< render::RenderTargetSet > m_impostorTargetDistance;
	Ref< render::RenderTargetSet > m_impostorTargetForward;
	Ref< render::VertexBuffer > m_vertexBuffer;
	Ref< render::IndexBuffer > m_indexBuffer;
	render::Primitives m_primitives;
	render::handle_t m_handleBillboardView;
	render::handle_t m_handleImpostorTargetForward;
	render::handle_t m_handleImpostorTargetDistance;

	CloudParticleCluster m_cluster;
	CloudParticleData m_particleData;

	Matrix44 m_transform;
	Vector4 m_lastLightDirection;
	Vector4 m_lastEyePosition;

	void renderCluster(
		render::RenderContext* renderContext,
		const world::WorldRenderView* worldRenderView,
		render::PrimitiveRenderer* primitiveRenderer,
		const CloudParticleCluster& cluster
	);
};

	}
}

#endif	// traktor_weather_CloudEntity_H
