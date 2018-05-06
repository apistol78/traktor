/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_animation_PathEntity_H
#define traktor_animation_PathEntity_H

#include "Core/Ref.h"
#include "Core/Math/TransformPath.h"
#include "World/Entity.h"

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

	struct IListener : public IRefCount
	{
		virtual void notifyPathFinished(PathEntity* entity) = 0;
	};

	PathEntity(const Transform& transform, const TransformPath& path, TimeMode timeMode, float timeOffset, world::Entity* entity);

	virtual ~PathEntity();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual bool getTransform(Transform& outTransform) const T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual void update(const world::UpdateParams& update) T_OVERRIDE T_FINAL;

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

	void setListener(IListener* listener) { m_listener = listener; }

private:
	Transform m_transform;
	TransformPath m_path;
	TimeMode m_timeMode;
	Ref< world::Entity > m_entity;
	float m_timeScale;
	float m_timeDeltaSign;
	float m_time;
	Ref< IListener > m_listener;
};

	}
}

#endif	// traktor_animation_PathEntity_H
