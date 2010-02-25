#include "I18N/Format.h"
#include "Sound/Resound/SequenceGrain.h"
#include "Sound/Editor/Resound/SequenceGrainFacade.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SequenceGrainFacade", SequenceGrainFacade, IGrainFacade)

int32_t SequenceGrainFacade::getImage(const IGrain* grain) const
{
	return 1;
}

std::wstring SequenceGrainFacade::getText(const IGrain* grain) const
{
	uint32_t count = checked_type_cast< const SequenceGrain* >(grain)->getGrains().size();
	return i18n::Format(L"RESOUND_SEQUENCE_GRAIN_TEXT", int32_t(count));
}

bool SequenceGrainFacade::canHaveChildren() const
{
	return true;
}

bool SequenceGrainFacade::addChild(IGrain* parentGrain, IGrain* childGrain)
{
	checked_type_cast< SequenceGrain*, false >(parentGrain)->addGrain(childGrain);
	return true;
}

bool SequenceGrainFacade::removeChild(IGrain* parentGrain, IGrain* childGrain)
{
	checked_type_cast< SequenceGrain*, false >(parentGrain)->removeGrain(childGrain);
	return true;
}

bool SequenceGrainFacade::getChildren(IGrain* grain, RefArray< IGrain >& outChildren)
{
	outChildren = checked_type_cast< SequenceGrain*, false >(grain)->getGrains();
	return true;
}

	}
}
