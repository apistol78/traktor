/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "I18N/Format.h"
#include "Sound/Resound/RandomGrainData.h"
#include "Sound/Editor/Resound/RandomGrainFacade.h"
#include "Ui/StyleBitmap.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.RandomGrainFacade", RandomGrainFacade, IGrainFacade)

RandomGrainFacade::RandomGrainFacade()
{
	m_image = new ui::StyleBitmap(L"Sound.RandomGrain");
}

ui::Widget* RandomGrainFacade::createView(IGrainData* grain, ui::Widget* parent)
{
	return nullptr;
}

ui::StyleBitmap* RandomGrainFacade::getImage(const IGrainData* grain) const
{
	return m_image;
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
