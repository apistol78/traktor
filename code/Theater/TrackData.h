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
	TrackData();

	void setEntityData(world::EntityData* entityData);

	Ref< world::EntityData > getEntityData() const;

	void setLookAtEntityData(world::EntityData* entityData);

	Ref< world::EntityData > getLookAtEntityData() const;

	const TransformPath& getPath() const;

	TransformPath& getPath();

	float getLoopStart() const;

	float getLoopEnd() const;

	float getTimeOffset() const;

	virtual bool serialize(ISerializer& s);

private:
	Ref< world::EntityData > m_entityData;
	Ref< world::EntityData > m_lookAtEntityData;
	TransformPath m_path;
	float m_loopStart;
	float m_loopEnd;
	float m_timeOffset;
};

	}
}

#endif	// traktor_theater_TrackData_H
