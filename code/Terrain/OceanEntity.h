#ifndef traktor_terrain_OceanEntity_H
#define traktor_terrain_OceanEntity_H

#include "Core/Math/Color4f.h"
#include "Core/Math/Vector4.h"
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
class ITexture;
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

class OceanEntityData;

class T_DLLCLASS OceanEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	enum { MaxWaves = 32 };

	OceanEntity();

	bool create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const OceanEntityData& data);

	virtual void destroy();

	void render(
		render::RenderContext* renderContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		bool reflectionEnable
	);

	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb3 getBoundingBox() const;

	virtual void update(const world::UpdateParams& update);

private:
	resource::Proxy< render::Shader > m_shader;
	resource::Proxy< render::ITexture > m_reflectionMap;
	Ref< render::IndexBuffer > m_indexBuffer;
	Ref< render::VertexBuffer > m_vertexBuffer;
	render::Primitives m_primitives;
	Transform m_transform;
	Color4f m_shallowTint;
	Color4f m_reflectionTint;
	Color4f m_deepColor;
	float m_opacity;
	bool m_allowSSReflections;
	Vector4 m_wavesA[4];
	Vector4 m_wavesB[4];
};

	}
}

#endif	// traktor_terrain_OceanEntity_H
