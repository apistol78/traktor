#ifndef traktor_animation_PathEntity_H
#define traktor_animation_PathEntity_H

#include "Core/Math/TransformPath.h"
#include "World/Entity/Entity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class IWorldRenderPass;
class WorldContext;
class WorldRenderView;

	}

	namespace animation
	{

/*! \brief Movement path entity.
 * \ingroup Animation
 */
class T_DLLCLASS PathEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	enum TimeMode
	{
		TmManual,
		TmOnce,
		TmLoop,
		TmPingPong
	};

	PathEntity(const Transform& transform, const TransformPath& path, TimeMode timeMode, world::Entity* entity);

	virtual ~PathEntity();

	virtual void destroy();

	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb3 getBoundingBox() const;

	virtual void update(const UpdateParams& update);

	void precull(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView
	);

	void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass
	);

	void setPath(const TransformPath& path) { m_path = path; }

	const TransformPath& getPath() const { return m_path; }

	void setTimeMode(TimeMode timeMode) { m_timeMode = timeMode; }

	TimeMode getTimeMode() const { return m_timeMode; }

	void setTimeScale(float timeScale) { m_timeScale = timeScale; }

	float getTimeScale() const { return m_timeScale; }

	void setTime(float time) { m_time = time; }

	float getTime() const { return m_time; }

	world::Entity* getEntity() { return m_entity; }

private:
	Transform m_transform;
	TransformPath m_path;
	TimeMode m_timeMode;
	Ref< world::Entity > m_entity;
	float m_timeScale;
	float m_timeDeltaSign;
	float m_time;
};

	}
}

#endif	// traktor_animation_PathEntity_H
