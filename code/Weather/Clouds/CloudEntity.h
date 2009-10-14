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

class IRenderSystem;
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
		render::IRenderSystem* renderSystem,
		const resource::Proxy< render::Shader >& particleShader,
		const resource::Proxy< render::Shader >& impostorShader,
		uint32_t impostorTargetResolution,
		uint32_t impostorSliceCount,
		float impostorSliceOffset,
		const CloudParticleData& particleData
	);

	void render(render::RenderContext* renderContext, const world::WorldRenderView* worldRenderView, render::PrimitiveRenderer* primitiveRenderer);

	virtual void update(const world::EntityUpdate* update);

	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb getBoundingBox() const;

private:
	resource::Proxy< render::Shader > m_particleShader;
	resource::Proxy< render::Shader > m_impostorShader;
	RefArray< render::RenderTargetSet > m_impostorTargets;
	Ref< render::VertexBuffer > m_vertexBuffer;
	Ref< render::IndexBuffer > m_indexBuffer;
	render::handle_t m_handleBillboardView;
	render::handle_t m_handleImpostorTarget;

	uint32_t m_impostorSliceCount;
	float m_impostorSliceOffset;

	CloudParticleCluster m_cluster;
	CloudParticleData m_particleData;

	Transform m_transform;
	Vector4 m_lastCameraPosition;
	Vector4 m_lastCameraDirection;

	float m_timeUntilUpdate;
	uint32_t m_updateCount;

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
