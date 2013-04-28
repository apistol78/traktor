#include "I18N/Format.h"
#include "Sound/Resound/MuteGrainData.h"
#include "Sound/Editor/Resound/MuteGrainFacade.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.MuteGrainFacade", MuteGrainFacade, IGrainFacade)

ui::Widget* MuteGrainFacade::createView(IGrainData* grain, ui::Widget* parent) const
{
	return 0;
}

int32_t MuteGrainFacade::getImage(const IGrainData* grain) const
{
	return 4;
}

std::wstring MuteGrainFacade::getText(const IGrainData* grain) const
{
	const MuteGrainData* muteGrain = static_cast< const MuteGrainData* >(grain);
	return i18n::Format(L"RESOUND_MUTE_GRAIN_TEXT", int32_t(muteGrain->getDuration() * 1000));
}

bool MuteGrainFacade::getProperties(const IGrainData* grain, std::set< std::wstring >& outProperties) const
{
	return false;
}

bool MuteGrainFacade::canHaveChildren() const
{
	return false;
}

bool MuteGrainFacade::addChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	return false;
}

bool MuteGrainFacade::removeChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	return false;
}

bool MuteGrainFacade::getChildren(IGrainData* grain, RefArray< IGrainData >& outChildren)
{
	return true;
}

	}
}
