#ifndef traktor_terrain_OceanEntity_H
#define traktor_terrain_OceanEntity_H

#include "Core/Math/Vector4.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "World/Entity/Entity.h"

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

class IRenderSystem;
class RenderContext;
class VertexBuffer;
class IndexBuffer;
class Shader;
class ScreenRenderer;

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

class OceanEntityData;
class Terrain;

class T_DLLCLASS OceanEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	enum { MaxWaves = 32 };

	OceanEntity();

	bool create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const OceanEntityData& data);

	void render(
		render::RenderContext* renderContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass
	);

	virtual Aabb3 getBoundingBox() const;

	virtual void update(const UpdateParams& update);

	float getAltitude() const { return m_altitude; }

private:
	resource::Proxy< Terrain > m_terrain;
	Ref< render::VertexBuffer > m_vertexBuffer;
	Ref< render::IndexBuffer > m_indexBuffer;
	Ref< render::ScreenRenderer > m_screenRenderer;
	render::Primitives m_primitives;
	resource::Proxy< render::Shader > m_shader;
	float m_altitude;
	Vector4 m_waveData[MaxWaves];
};

	}
}

#endif	// traktor_terrain_OceanEntity_H
