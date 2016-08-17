#ifndef traktor_terrain_OceanComponent_H
#define traktor_terrain_OceanComponent_H

#include "Core/Math/Color4f.h"
#include "Core/Math/Vector4.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "World/IEntityComponent.h"

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

class Entity;
class IWorldRenderPass;
class WorldRenderView;

	}

	namespace terrain
	{

class OceanComponentData;
class Terrain;

/*! \brief Ocean component.
 * \ingroup Terrain
 */
class T_DLLCLASS OceanComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	enum { MaxWaves = 32 };

	OceanComponent();

	virtual ~OceanComponent();

	bool create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const OceanComponentData& data);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setOwner(world::Entity* owner) T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual void update(const world::UpdateParams& update) T_OVERRIDE T_FINAL;

	void render(
		render::RenderContext* renderContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		bool reflectionEnable
	);

	void setShallowTint(const Color4f& shallowTint) { m_shallowTint = shallowTint; }

	const Color4f& getShallowTint() const { return m_shallowTint; }

	void setReflectionTint(const Color4f& reflectionTint) { m_reflectionTint = reflectionTint; }

	const Color4f& getReflectionTint() const { return m_reflectionTint; }

	void setShadowTint(const Color4f& shadowTint) { m_shadowTint = shadowTint; }

	const Color4f& getShadowTint() const { return m_shadowTint; }

	void setDeepColor(const Color4f& deepColor) { m_deepColor = deepColor; }

	const Color4f& getDeepColor() const { return m_deepColor; }

	void setOpacity(float opacity) { m_opacity = opacity; }

	float getOpacity() const { return m_opacity; }

	float getMaxAmplitude() const { return m_maxAmplitude; }

private:
	world::Entity* m_owner;
	resource::Proxy< render::Shader > m_shader;
	resource::Proxy< render::ITexture > m_reflectionMap;
	resource::Proxy< Terrain > m_terrain;
	Ref< render::IndexBuffer > m_indexBuffer;
	Ref< render::VertexBuffer > m_vertexBuffer;
	render::Primitives m_primitives;
	Color4f m_shallowTint;
	Color4f m_reflectionTint;
	Color4f m_shadowTint;
	Color4f m_deepColor;
	float m_opacity;
	float m_maxAmplitude;
	bool m_allowSSReflections;
	Vector4 m_wavesA[4];
	Vector4 m_wavesB[4];
};

	}
}

#endif	// traktor_terrain_OceanComponent_H
