#ifndef traktor_animation_PathEntity_H
#define traktor_animation_PathEntity_H

#include "Core/Heap/Ref.h"
#include "Core/Math/TransformPath.h"
#include "World/Entity/SpatialEntity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class WorldContext;
class WorldRenderView;

	}

	namespace animation
	{

/*! \brief Movement path entity.
 * \ingroup Animation
 */
class T_DLLCLASS PathEntity : public world::SpatialEntity
{
	T_RTTI_CLASS(PathEntity)

public:
	enum TimeMode
	{
		TmManual,
		TmOnce,
		TmLoop,
		TmPingPong
	};

	PathEntity(const Matrix44& transform, const TransformPath& path, TimeMode timeMode, world::SpatialEntity* entity);

	virtual ~PathEntity();

	virtual void destroy();

	virtual void setTransform(const Matrix44& transform);

	virtual bool getTransform(Matrix44& outTransform) const;

	virtual Aabb getBoundingBox() const;

	virtual void update(const world::EntityUpdate* update);

	void render(world::WorldContext* worldContext, world::WorldRenderView* worldRenderView);

	void setPath(const TransformPath& path) { m_path = path; }

	const TransformPath& getPath() const { return m_path; }

	void setTimeMode(TimeMode timeMode) { m_timeMode = timeMode; }

	TimeMode getTimeMode() const { return m_timeMode; }

	void setTimeScale(float timeScale) { m_timeScale = timeScale; }

	float getTimeScale() const { return m_timeScale; }

	void setTime(float time) { m_time = time; }

	float getTime() const { return m_time; }

	world::SpatialEntity* getEntity() { return m_entity; }

private:
	Matrix44 m_transform;
	TransformPath m_path;
	TimeMode m_timeMode;
	Ref< world::SpatialEntity > m_entity;
	float m_timeScale;
	float m_timeDeltaSign;
	float m_time;
};

	}
}

#endif	// traktor_animation_PathEntity_H
