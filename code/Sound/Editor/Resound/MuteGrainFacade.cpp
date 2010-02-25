#include "I18N/Format.h"
#include "Sound/Resound/MuteGrain.h"
#include "Sound/Editor/Resound/MuteGrainFacade.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.MuteGrainFacade", MuteGrainFacade, IGrainFacade)

int32_t MuteGrainFacade::getImage(const IGrain* grain) const
{
	return 4;
}

std::wstring MuteGrainFacade::getText(const IGrain* grain) const
{
	const MuteGrain* muteGrain = static_cast< const MuteGrain* >(grain);
	return i18n::Format(L"RESOUND_MUTE_GRAIN_TEXT", int32_t(muteGrain->getDuration() * 1000));
}

bool MuteGrainFacade::canHaveChildren() const
{
	return false;
}

bool MuteGrainFacade::addChild(IGrain* parentGrain, IGrain* childGrain)
{
	return false;
}

bool MuteGrainFacade::removeChild(IGrain* parentGrain, IGrain* childGrain)
{
	return false;
}

bool MuteGrainFacade::getChildren(IGrain* grain, RefArray< IGrain >& outChildren)
{
	return true;
}

	}
}
