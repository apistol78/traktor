#include "I18N/Format.h"
#include "I18N/Text.h"
#include "Sound/Resound/RepeatGrain.h"
#include "Sound/Editor/Resound/RepeatGrainFacade.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.RepeatGrainFacade", RepeatGrainFacade, IGrainFacade)

int32_t RepeatGrainFacade::getImage(const IGrain* grain) const
{
	return 2;
}

std::wstring RepeatGrainFacade::getText(const IGrain* grain) const
{
	const RepeatGrain* repeatGrain = static_cast< const RepeatGrain* >(grain);
	if (repeatGrain->getCount() != 0)
		return i18n::Format(L"RESOUND_REPEAT_GRAIN_TEXT", int32_t(repeatGrain->getCount()));
	else
		return i18n::Text(L"RESOUND_REPEAT_GRAIN_INFINITE_TEXT");
}

bool RepeatGrainFacade::canHaveChildren() const
{
	return false;
}

bool RepeatGrainFacade::addChild(IGrain* parentGrain, IGrain* childGrain)
{
	return false;
}

bool RepeatGrainFacade::removeChild(IGrain* parentGrain, IGrain* childGrain)
{
	return false;
}

bool RepeatGrainFacade::getChildren(IGrain* grain, RefArray< IGrain >& outChildren)
{
	outChildren.push_back(checked_type_cast< RepeatGrain*, false >(grain)->getGrain());
	return true;
}

	}
}
