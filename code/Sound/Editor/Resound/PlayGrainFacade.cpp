#include "I18N/Format.h"
#include "Sound/Resound/PlayGrain.h"
#include "Sound/Editor/Resound/PlayGrainFacade.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.PlayGrainFacade", PlayGrainFacade, IGrainFacade)

int32_t PlayGrainFacade::getImage(const IGrain* grain) const
{
	return 0;
}

std::wstring PlayGrainFacade::getText(const IGrain* grain) const
{
	const PlayGrain* playGrain = static_cast< const PlayGrain* >(grain);
	return i18n::Format(
		L"RESOUND_PLAY_GRAIN_TEXT",
		int32_t((playGrain->getGain().min + 1.0f) * 100),
		int32_t((playGrain->getGain().max + 1.0f) * 100)
	);
}

bool PlayGrainFacade::canHaveChildren() const
{
	return false;
}

bool PlayGrainFacade::addChild(IGrain* parentGrain, IGrain* childGrain)
{
	return false;
}

bool PlayGrainFacade::removeChild(IGrain* parentGrain, IGrain* childGrain)
{
	return false;
}

bool PlayGrainFacade::getChildren(IGrain* grain, RefArray< IGrain >& outChildren)
{
	return true;
}

	}
}
