#ifndef traktor_theater_Track_H
#define traktor_theater_Track_H

#include "Core/Heap/Ref.h"
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
	T_RTTI_CLASS(Track)

public:
	Track(world::SpatialEntity* entity, const TransformPath& path);

	void setEntity(world::SpatialEntity* entity);

	Ref< world::SpatialEntity > getEntity() const;

	const TransformPath& getPath() const;

	TransformPath& getPath();

private:
	Ref< world::SpatialEntity > m_entity;
	TransformPath m_path;
};

	}
}

#endif	// traktor_theater_Track_H
