/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_animation_PathComponent_H
#define traktor_animation_PathComponent_H

#include "Core/Math/TransformPath.h"
#include "World/IEntityComponent.h"

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

class Entity;

	}

	namespace animation
	{

/*! \brief
 * \ingroup Animation
 */
class T_DLLCLASS PathComponent : public world::IEntityComponent
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

	PathComponent(
		const TransformPath& path,
		TimeMode timeMode,
		float timeOffset
	);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setOwner(world::Entity* owner) T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual void update(const world::UpdateParams& update) T_OVERRIDE T_FINAL;

private:
	world::Entity* m_owner;
	Transform m_transform;
	TransformPath m_path;
	TimeMode m_timeMode;
	float m_timeScale;
	float m_timeDeltaSign;
	float m_time;
};

	}
}

#endif	// traktor_animation_PathComponent_H
