#ifndef traktor_theater_TrackData_H
#define traktor_theater_TrackData_H

#include "Core/Math/TransformPath.h"
#include "Core/Serialization/ISerializable.h"

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

class EntityData;

	}

	namespace theater
	{

class T_DLLCLASS TrackData : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setEntityData(world::EntityData* entityData);

	Ref< world::EntityData > getEntityData() const;

	const TransformPath& getPath() const;

	TransformPath& getPath();

	virtual bool serialize(ISerializer& s);

private:
	Ref< world::EntityData > m_entityData;
	TransformPath m_path;
};

	}
}

#endif	// traktor_theater_TrackData_H
