/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_theater_Track_H
#define traktor_theater_Track_H

#include "Core/Object.h"
#include "Core/Math/TransformPath.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_THEATER_EXPORT)
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

	namespace theater
	{

/*! \brief Track
 * \ingroup Theater
 */
class T_DLLCLASS Track : public Object
{
	T_RTTI_CLASS;

public:
	Track(
		world::Entity* entity,
		world::Entity* lookAtEntity,
		const TransformPath& path,
		float loopStart,
		float loopEnd,
		float timeOffset,
		float wobbleMagnitude,
		float wobbleRate
	);

	void setEntity(world::Entity* entity);

	Ref< world::Entity > getEntity() const;

	void setLookAtEntity(world::Entity* entity);

	Ref< world::Entity > getLookAtEntity() const;

	const TransformPath& getPath() const;

	TransformPath& getPath();

	float getLoopStart() const;

	float getLoopEnd() const;

	float getTimeOffset() const;

	float getWobbleMagnitude() const;

	float getWobbleRate() const;

private:
	Ref< world::Entity > m_entity;
	Ref< world::Entity > m_lookAtEntity;
	TransformPath m_path;
	float m_loopStart;
	float m_loopEnd;
	float m_timeOffset;
	float m_wobbleMagnitude;
	float m_wobbleRate;
};

	}
}

#endif	// traktor_theater_Track_H
