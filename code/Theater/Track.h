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
		float timeOffset
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

private:
	Ref< world::Entity > m_entity;
	Ref< world::Entity > m_lookAtEntity;
	TransformPath m_path;
	float m_loopStart;
	float m_loopEnd;
	float m_timeOffset;
};

	}
}

#endif	// traktor_theater_Track_H
