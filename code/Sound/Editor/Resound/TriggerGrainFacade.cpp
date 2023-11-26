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
#include "Sound/Resound/TriggerGrainData.h"
#include "Sound/Editor/Resound/TriggerGrainFacade.h"
#include "Ui/StyleBitmap.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.TriggerGrainFacade", TriggerGrainFacade, IGrainFacade)

TriggerGrainFacade::TriggerGrainFacade()
{
	m_image = new ui::StyleBitmap(L"Sound.TriggerGrain");
}

ui::Widget* TriggerGrainFacade::createView(IGrainData* grain, ui::Widget* parent)
{
	return nullptr;
}

ui::StyleBitmap* TriggerGrainFacade::getImage(const IGrainData* grain) const
{
	return m_image;
}

std::wstring TriggerGrainFacade::getText(const IGrainData* grain) const
{
	return i18n::Text(L"RESOUND_TRIGGER_GRAIN_TEXT");
}

bool TriggerGrainFacade::getProperties(const IGrainData* grain, std::set< std::wstring >& outProperties) const
{
	const TriggerGrainData* triggerGrain = mandatory_non_null_type_cast< const TriggerGrainData* >(grain);
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
	TriggerGrainData* triggerGrain = mandatory_non_null_type_cast< TriggerGrainData* >(grain);
	outChildren.push_back(triggerGrain->getGrain());
	return true;
}

}
