/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

/*! \brief Theater scene controller data.
 * \ingroup Theater
 */
class T_DLLCLASS TheaterControllerData : public scene::ISceneControllerData
{
	T_RTTI_CLASS;

public:
	TheaterControllerData();

	virtual Ref< scene::ISceneController > createController(const std::map< const world::EntityData*, Ref< world::Entity > >& entityProducts, bool editor) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const RefArray< ActData >& getActs() const { return m_acts; }

	RefArray< ActData >& getActs() { return m_acts; }

	float getActStartTime(int32_t act) const;

private:
	friend class TheaterControllerPipeline;

	RefArray< ActData > m_acts;
	bool m_repeatActs;
	bool m_randomizeActs;
};

	}
}

#endif	// traktor_theater_TheaterControllerData_H
