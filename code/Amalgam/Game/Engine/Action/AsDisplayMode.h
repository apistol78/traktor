#ifndef traktor_amalgam_AsDisplayMode_H
#define traktor_amalgam_AsDisplayMode_H

#include "Core/RefArray.h"
#include "Flash/Action/ActionObjectRelay.h"
#include "Render/Types.h"

namespace traktor
{
	namespace amalgam
	{

class IEnvironment;

	}

	namespace amalgam
	{

/*! \brief
 * \ingroup Amalgam
 */
class AsDisplayMode : public flash::ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	AsDisplayMode();

	AsDisplayMode(const render::DisplayMode& displayMode);

	static RefArray< AsDisplayMode > getAvailableModes(IEnvironment* environment);

	static Ref< AsDisplayMode > getCurrentMode(IEnvironment* environment);

	const render::DisplayMode& getDisplayMode() const { return m_displayMode; }

private:
	render::DisplayMode m_displayMode;
};

	}
}

#endif	// traktor_amalgam_AsDisplayMode_H
