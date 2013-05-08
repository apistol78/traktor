#ifndef traktor_theater_TheaterControllerData_H
#define traktor_theater_TheaterControllerData_H

#include "Core/RefArray.h"
#include "Scene/ISceneControllerData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_THEATER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace theater
	{

class TrackData;

class T_DLLCLASS TheaterControllerData : public scene::ISceneControllerData
{
	T_RTTI_CLASS;

public:
	TheaterControllerData();

	virtual Ref< scene::ISceneController > createController(const std::map< const world::EntityData*, Ref< world::Entity > >& entityProducts) const;

	virtual void serialize(ISerializer& s);

	float getDuration() const { return m_duration; }

	RefArray< TrackData >& getTrackData() { return m_trackData; }

private:
	float m_duration;
	RefArray< TrackData > m_trackData;
};

	}
}

#endif	// traktor_theater_TheaterControllerData_H
