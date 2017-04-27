/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "I18N/Format.h"
#include "I18N/Text.h"
#include "Sound/Resound/InLoopOutGrainData.h"
#include "Sound/Editor/Resound/InLoopOutGrainFacade.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.InLoopOutGrainFacade", InLoopOutGrainFacade, IGrainFacade)

ui::Widget* InLoopOutGrainFacade::createView(IGrainData* grain, ui::Widget* parent)
{
	return 0;
}

int32_t InLoopOutGrainFacade::getImage(const IGrainData* grain) const
{
	return 2;
}

std::wstring InLoopOutGrainFacade::getText(const IGrainData* grain) const
{
	return i18n::Text(L"RESOUND_INLOOPOUT_GRAIN_TEXT");
}

bool InLoopOutGrainFacade::getProperties(const IGrainData* grain, std::set< std::wstring >& outProperties) const
{
	const InLoopOutGrainData* iloGrain = checked_type_cast< const InLoopOutGrainData*, false >(grain);
	outProperties.insert(iloGrain->getId());
	return true;
}

bool InLoopOutGrainFacade::canHaveChildren() const
{
	return false;
}

bool InLoopOutGrainFacade::addChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	return false;
}

bool InLoopOutGrainFacade::removeChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	return false;
}

bool InLoopOutGrainFacade::getChildren(IGrainData* grain, RefArray< IGrainData >& outChildren)
{
	InLoopOutGrainData* iloGrain = checked_type_cast< InLoopOutGrainData*, false >(grain);
	outChildren.push_back(iloGrain->getInGrain());
	outChildren.push_back(iloGrain->getInLoopGrain());
	outChildren.push_back(iloGrain->getOutGrain());
	outChildren.push_back(iloGrain->getOutLoopGrain());
	return true;
}

	}
}
