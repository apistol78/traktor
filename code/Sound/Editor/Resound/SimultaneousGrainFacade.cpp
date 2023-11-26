/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "I18N/Format.h"
#include "Sound/Resound/SimultaneousGrainData.h"
#include "Sound/Editor/Resound/SimultaneousGrainFacade.h"
#include "Ui/StyleBitmap.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SimultaneousGrainFacade", SimultaneousGrainFacade, IGrainFacade)

SimultaneousGrainFacade::SimultaneousGrainFacade()
{
	m_image = new ui::StyleBitmap(L"Sound.SimultaneousGrain");
}

ui::Widget* SimultaneousGrainFacade::createView(IGrainData* grain, ui::Widget* parent)
{
	return nullptr;
}

ui::StyleBitmap* SimultaneousGrainFacade::getImage(const IGrainData* grain) const
{
	return m_image;
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
	mandatory_non_null_type_cast< SimultaneousGrainData* >(parentGrain)->addGrain(childGrain);
	return true;
}

bool SimultaneousGrainFacade::removeChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	mandatory_non_null_type_cast< SimultaneousGrainData* >(parentGrain)->removeGrain(childGrain);
	return true;
}

bool SimultaneousGrainFacade::getChildren(IGrainData* grain, RefArray< IGrainData >& outChildren)
{
	outChildren = mandatory_non_null_type_cast< SimultaneousGrainData* >(grain)->getGrains();
	return true;
}

}
