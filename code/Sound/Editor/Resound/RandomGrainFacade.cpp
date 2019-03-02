#include "I18N/Format.h"
#include "Sound/Resound/RandomGrainData.h"
#include "Sound/Editor/Resound/RandomGrainFacade.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.RandomGrainFacade", RandomGrainFacade, IGrainFacade)

ui::Widget* RandomGrainFacade::createView(IGrainData* grain, ui::Widget* parent)
{
	return 0;
}

int32_t RandomGrainFacade::getImage(const IGrainData* grain) const
{
	return 3;
}

std::wstring RandomGrainFacade::getText(const IGrainData* grain) const
{
	uint32_t count = checked_type_cast< const RandomGrainData* >(grain)->getGrains().size();
	return i18n::Format(L"RESOUND_RANDOM_GRAIN_TEXT", int32_t(count));
}

bool RandomGrainFacade::getProperties(const IGrainData* grain, std::set< std::wstring >& outProperties) const
{
	return false;
}

bool RandomGrainFacade::canHaveChildren() const
{
	return true;
}

bool RandomGrainFacade::addChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	checked_type_cast< RandomGrainData*, false >(parentGrain)->addGrain(childGrain);
	return true;
}

bool RandomGrainFacade::removeChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	checked_type_cast< RandomGrainData*, false >(parentGrain)->removeGrain(childGrain);
	return true;
}

bool RandomGrainFacade::getChildren(IGrainData* grain, RefArray< IGrainData >& outChildren)
{
	outChildren = checked_type_cast< RandomGrainData*, false >(grain)->getGrains();
	return true;
}

	}
}
