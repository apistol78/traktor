#pragma once

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

/*! Theater scene controller data.
 * \ingroup Theater
 */
class T_DLLCLASS TheaterControllerData : public scene::ISceneControllerData
{
	T_RTTI_CLASS;

public:
	TheaterControllerData();

	virtual Ref< scene::ISceneController > createController(const SmallMap< Guid, Ref< world::Entity > >& entityProducts, bool editor) const override final;

	virtual void serialize(ISerializer& s) override final;

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

