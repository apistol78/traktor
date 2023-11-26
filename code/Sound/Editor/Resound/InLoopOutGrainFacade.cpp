/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "I18N/Format.h"
#include "I18N/Text.h"
#include "Sound/Resound/InLoopOutGrainData.h"
#include "Sound/Editor/Resound/InLoopOutGrainFacade.h"
#include "Ui/StyleBitmap.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.InLoopOutGrainFacade", InLoopOutGrainFacade, IGrainFacade)

InLoopOutGrainFacade::InLoopOutGrainFacade()
{
	m_image = new ui::StyleBitmap(L"Sound.InLoopOutGrain");
}

ui::Widget* InLoopOutGrainFacade::createView(IGrainData* grain, ui::Widget* parent)
{
	return nullptr;
}

ui::StyleBitmap* InLoopOutGrainFacade::getImage(const IGrainData* grain) const
{
	return m_image;
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
