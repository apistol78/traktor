#ifndef traktor_theater_ActData_H
#define traktor_theater_ActData_H

#include <map>
#include "Core/RefArray.h"
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

class Entity;
class EntityData;

	}

	namespace theater
	{

class Act;
class TrackData;

/*! \brief Act description.
 * \ingroup Theater
 */
class T_DLLCLASS ActData : public ISerializable
{
	T_RTTI_CLASS;

public:
	ActData();

	Ref< Act > createInstance(const std::map< const world::EntityData*, Ref< world::Entity > >& entityProducts) const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	void setName(const std::wstring& name) { m_name = name; }

	const std::wstring& getName() const { return m_name; }

	void setDuration(float duration) { m_duration = duration; }

	float getDuration() const { return m_duration; }

	const RefArray< TrackData >& getTracks() const { return m_tracks; }

	RefArray< TrackData >& getTracks() { return m_tracks; }

private:
	std::wstring m_name;
	float m_duration;
	RefArray< TrackData > m_tracks;
};

	}
}

#endif	// traktor_theater_ActData_H
