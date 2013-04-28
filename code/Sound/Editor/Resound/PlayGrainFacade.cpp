#include "I18N/Format.h"
#include "Sound/Resound/PlayGrainData.h"
#include "Sound/Editor/Resound/PlayGrainFacade.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.PlayGrainFacade", PlayGrainFacade, IGrainFacade)

ui::Widget* PlayGrainFacade::createView(IGrainData* grain, ui::Widget* parent) const
{
	return 0;
}

int32_t PlayGrainFacade::getImage(const IGrainData* grain) const
{
	return 0;
}

std::wstring PlayGrainFacade::getText(const IGrainData* grain) const
{
	const PlayGrainData* playGrain = static_cast< const PlayGrainData* >(grain);
	return i18n::Format(
		L"RESOUND_PLAY_GRAIN_TEXT",
		int32_t((playGrain->getGain().min + 1.0f) * 100),
		int32_t((playGrain->getGain().max + 1.0f) * 100)
	);
}

bool PlayGrainFacade::getProperties(const IGrainData* grain, std::set< std::wstring >& outProperties) const
{
	return false;
}

bool PlayGrainFacade::canHaveChildren() const
{
	return false;
}

bool PlayGrainFacade::addChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	return false;
}

bool PlayGrainFacade::removeChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	return false;
}

bool PlayGrainFacade::getChildren(IGrainData* grain, RefArray< IGrainData >& outChildren)
{
	return true;
}

	}
}
