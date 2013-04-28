#include "I18N/Format.h"
#include "I18N/Text.h"
#include "Sound/Resound/TriggerGrainData.h"
#include "Sound/Editor/Resound/TriggerGrainFacade.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.TriggerGrainFacade", TriggerGrainFacade, IGrainFacade)

ui::Widget* TriggerGrainFacade::createView(IGrainData* grain, ui::Widget* parent) const
{
	return 0;
}

int32_t TriggerGrainFacade::getImage(const IGrainData* grain) const
{
	return 2;
}

std::wstring TriggerGrainFacade::getText(const IGrainData* grain) const
{
	return i18n::Text(L"RESOUND_TRIGGER_GRAIN_TEXT");
}

bool TriggerGrainFacade::getProperties(const IGrainData* grain, std::set< std::wstring >& outProperties) const
{
	const TriggerGrainData* triggerGrain = checked_type_cast< const TriggerGrainData*, false >(grain);
	outProperties.insert(triggerGrain->getId());
	return true;
}

bool TriggerGrainFacade::canHaveChildren() const
{
	return false;
}

bool TriggerGrainFacade::addChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	return false;
}

bool TriggerGrainFacade::removeChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	return false;
}

bool TriggerGrainFacade::getChildren(IGrainData* grain, RefArray< IGrainData >& outChildren)
{
	TriggerGrainData* triggerGrain = checked_type_cast< TriggerGrainData*, false >(grain);
	outChildren.push_back(triggerGrain->getGrain());
	return true;
}

	}
}
