#ifndef traktor_theater_TrackData_H
#define traktor_theater_TrackData_H

#include "Core/Serialization/ISerializable.h"
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

class EntityInstance;

	}

	namespace theater
	{

class T_DLLCLASS TrackData : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setInstance(world::EntityInstance* instance);

	Ref< world::EntityInstance > getInstance() const;

	const TransformPath& getPath() const;

	TransformPath& getPath();

	virtual bool serialize(ISerializer& s);

private:
	Ref< world::EntityInstance > m_instance;
	TransformPath m_path;
};

	}
}

#endif	// traktor_theater_TrackData_H
