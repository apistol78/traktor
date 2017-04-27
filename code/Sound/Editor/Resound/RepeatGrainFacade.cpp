/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "I18N/Format.h"
#include "I18N/Text.h"
#include "Sound/Resound/RepeatGrainData.h"
#include "Sound/Editor/Resound/RepeatGrainFacade.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.RepeatGrainFacade", RepeatGrainFacade, IGrainFacade)

ui::Widget* RepeatGrainFacade::createView(IGrainData* grain, ui::Widget* parent)
{
	return 0;
}

int32_t RepeatGrainFacade::getImage(const IGrainData* grain) const
{
	return 2;
}

std::wstring RepeatGrainFacade::getText(const IGrainData* grain) const
{
	const RepeatGrainData* repeatGrain = static_cast< const RepeatGrainData* >(grain);
	if (repeatGrain->getCount() != 0)
		return i18n::Format(L"RESOUND_REPEAT_GRAIN_TEXT", int32_t(repeatGrain->getCount()));
	else
		return i18n::Text(L"RESOUND_REPEAT_GRAIN_INFINITE_TEXT");
}

bool RepeatGrainFacade::getProperties(const IGrainData* grain, std::set< std::wstring >& outProperties) const
{
	return false;
}

bool RepeatGrainFacade::canHaveChildren() const
{
	return false;
}

bool RepeatGrainFacade::addChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	return false;
}

bool RepeatGrainFacade::removeChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	return false;
}

bool RepeatGrainFacade::getChildren(IGrainData* grain, RefArray< IGrainData >& outChildren)
{
	RepeatGrainData* repeatGrain = checked_type_cast< RepeatGrainData*, false >(grain);
	outChildren.push_back(repeatGrain->getGrain());
	return true;
}

	}
}
