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

class ActData;

class T_DLLCLASS TheaterControllerData : public scene::ISceneControllerData
{
	T_RTTI_CLASS;

public:
	virtual Ref< scene::ISceneController > createController(const std::map< const world::EntityData*, Ref< world::Entity > >& entityProducts) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const RefArray< ActData >& getActs() const { return m_acts; }

	RefArray< ActData >& getActs() { return m_acts; }

private:
	RefArray< ActData > m_acts;
};

	}
}

#endif	// traktor_theater_TheaterControllerData_H
