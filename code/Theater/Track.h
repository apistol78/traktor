#ifndef traktor_theater_Track_H
#define traktor_theater_Track_H

#include "Core/Object.h"
#include "Core/Math/TransformPath.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_THEATER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class SpatialEntity;

	}

	namespace theater
	{

class T_DLLCLASS Track : public Object
{
	T_RTTI_CLASS;

public:
	Track(
		world::SpatialEntity* entity,
		world::SpatialEntity* lookAtEntity,
		const TransformPath& path,
		float loopStart,
		float loopEnd,
		float loopEase
	);

	void setEntity(world::SpatialEntity* entity);

	Ref< world::SpatialEntity > getEntity() const;

	void setLookAtEntity(world::SpatialEntity* entity);

	Ref< world::SpatialEntity > getLookAtEntity() const;

	const TransformPath& getPath() const;

	TransformPath& getPath();

	float getLoopStart() const;

	float getLoopEnd() const;

	float getLoopEase() const;

private:
	Ref< world::SpatialEntity > m_entity;
	Ref< world::SpatialEntity > m_lookAtEntity;
	TransformPath m_path;
	float m_loopStart;
	float m_loopEnd;
	float m_loopEase;
};

	}
}

#endif	// traktor_theater_Track_H
