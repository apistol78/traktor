#ifndef traktor_terrain_OceanEntity_H
#define traktor_terrain_OceanEntity_H

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
class RenderContext;
class ScreenRenderer;
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
	resource::Proxy< render::Shader > m_shaderComposite;
	Ref< render::ScreenRenderer > m_screenRenderer;
	Transform m_transform;
};

	}
}

#endif	// traktor_terrain_OceanEntity_H
