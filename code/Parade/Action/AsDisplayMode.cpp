#include "Parade/Action/AsDisplayMode.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.AsDisplayMode", AsDisplayMode, flash::ActionObjectRelay)

AsDisplayMode::AsDisplayMode()
:	flash::ActionObjectRelay("traktor.parade.DisplayMode")
{
}

RefArray< AsDisplayMode > AsDisplayMode::getAvailableModes(amalgam::IEnvironment* environment)
{
	RefArray< AsDisplayMode > modes;
	return modes;
}

	}
}
