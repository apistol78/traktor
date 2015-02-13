#include "Amalgam/IEnvironment.h"
#include "Amalgam/Engine/Action/AsDisplayMode.h"
#include "Render/IRenderSystem.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.AsDisplayMode", AsDisplayMode, flash::ActionObjectRelay)

AsDisplayMode::AsDisplayMode()
:	flash::ActionObjectRelay("traktor.amalgam.DisplayMode")
{
}

AsDisplayMode::AsDisplayMode(const render::DisplayMode& displayMode)
:	flash::ActionObjectRelay("traktor.amalgam.DisplayMode")
,	m_displayMode(displayMode)
{
}

RefArray< AsDisplayMode > AsDisplayMode::getAvailableModes(amalgam::IEnvironment* environment)
{
	render::IRenderSystem* renderSystem = environment->getRender()->getRenderSystem();
	T_ASSERT (renderSystem);

	uint32_t displayModeCount = renderSystem->getDisplayModeCount();

	RefArray< AsDisplayMode > displayModes;
	displayModes.reserve(displayModeCount);

	for (uint32_t i = 0; i < displayModeCount; ++i)
	{
		displayModes.push_back(new AsDisplayMode(
			renderSystem->getDisplayMode(i)
		));
	}

	return displayModes;
}

Ref< AsDisplayMode > AsDisplayMode::getCurrentMode(amalgam::IEnvironment* environment)
{
	render::IRenderSystem* renderSystem = environment->getRender()->getRenderSystem();
	T_ASSERT (renderSystem);

	return new AsDisplayMode(renderSystem->getCurrentDisplayMode());
}

	}
}
