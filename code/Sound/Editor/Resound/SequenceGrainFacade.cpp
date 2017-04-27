/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "I18N/Format.h"
#include "Sound/Resound/SequenceGrainData.h"
#include "Sound/Editor/Resound/SequenceGrainFacade.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SequenceGrainFacade", SequenceGrainFacade, IGrainFacade)

ui::Widget* SequenceGrainFacade::createView(IGrainData* grain, ui::Widget* parent)
{
	return 0;
}

int32_t SequenceGrainFacade::getImage(const IGrainData* grain) const
{
	return 1;
}

std::wstring SequenceGrainFacade::getText(const IGrainData* grain) const
{
	uint32_t count = checked_type_cast< const SequenceGrainData* >(grain)->getGrains().size();
	return i18n::Format(L"RESOUND_SEQUENCE_GRAIN_TEXT", int32_t(count));
}

bool SequenceGrainFacade::getProperties(const IGrainData* grain, std::set< std::wstring >& outProperties) const
{
	return false;
}

bool SequenceGrainFacade::canHaveChildren() const
{
	return true;
}

bool SequenceGrainFacade::addChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	checked_type_cast< SequenceGrainData*, false >(parentGrain)->addGrain(childGrain);
	return true;
}

bool SequenceGrainFacade::removeChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	checked_type_cast< SequenceGrainData*, false >(parentGrain)->removeGrain(childGrain);
	return true;
}

bool SequenceGrainFacade::getChildren(IGrainData* grain, RefArray< IGrainData >& outChildren)
{
	outChildren = checked_type_cast< SequenceGrainData*, false >(grain)->getGrains();
	return true;
}

	}
}
