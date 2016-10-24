#ifndef traktor_weather_PrecipitationComponent_H
#define traktor_weather_PrecipitationComponent_H

#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class StaticMesh;

	}

	namespace world
	{

class IWorldRenderPass;
class WorldContext;
class WorldRenderView;

	}

	namespace weather
	{

/*! \brief Precipitation component.
 * \ingroup Weather
 */
class T_DLLCLASS PrecipitationComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	PrecipitationComponent(const resource::Proxy< mesh::StaticMesh >& mesh);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setOwner(world::Entity* owner) T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual void update(const world::UpdateParams& update) T_OVERRIDE T_FINAL;

	void render(world::WorldContext& worldContext, world::WorldRenderView& worldRenderView, world::IWorldRenderPass& worldRenderPass);

private:
	resource::Proxy< mesh::StaticMesh > m_mesh;
	Vector4 m_lastEyePosition;
	Quaternion m_rotation;
};

	}
}

#endif	// traktor_weather_PrecipitationComponent_H
