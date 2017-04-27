/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "I18N/Format.h"
#include "Sound/Resound/SimultaneousGrainData.h"
#include "Sound/Editor/Resound/SimultaneousGrainFacade.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SimultaneousGrainFacade", SimultaneousGrainFacade, IGrainFacade)

ui::Widget* SimultaneousGrainFacade::createView(IGrainData* grain, ui::Widget* parent)
{
	return 0;
}

int32_t SimultaneousGrainFacade::getImage(const IGrainData* grain) const
{
	return 6;
}

std::wstring SimultaneousGrainFacade::getText(const IGrainData* grain) const
{
	uint32_t count = checked_type_cast< const SimultaneousGrainData* >(grain)->getGrains().size();
	return i18n::Format(L"RESOUND_SIMULTANEOUS_GRAIN_TEXT", int32_t(count));
}

bool SimultaneousGrainFacade::getProperties(const IGrainData* grain, std::set< std::wstring >& outProperties) const
{
	return false;
}

bool SimultaneousGrainFacade::canHaveChildren() const
{
	return true;
}

bool SimultaneousGrainFacade::addChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	checked_type_cast< SimultaneousGrainData*, false >(parentGrain)->addGrain(childGrain);
	return true;
}

bool SimultaneousGrainFacade::removeChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	checked_type_cast< SimultaneousGrainData*, false >(parentGrain)->removeGrain(childGrain);
	return true;
}

bool SimultaneousGrainFacade::getChildren(IGrainData* grain, RefArray< IGrainData >& outChildren)
{
	outChildren = checked_type_cast< SimultaneousGrainData*, false >(grain)->getGrains();
	return true;
}

	}
}
