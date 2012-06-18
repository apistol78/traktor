#ifndef traktor_parade_AsDisplayMode_H
#define traktor_parade_AsDisplayMode_H

#include "Core/RefArray.h"
#include "Flash/Action/ActionObjectRelay.h"
#include "Render/Types.h"

namespace traktor
{
	namespace amalgam
	{

class IEnvironment;

	}

	namespace parade
	{

class AsDisplayMode : public flash::ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	AsDisplayMode();

	static RefArray< AsDisplayMode > getAvailableModes(amalgam::IEnvironment* environment);

	render::DisplayMode dm;
};

	}
}

#endif	// traktor_parade_AsDisplayMode_H
