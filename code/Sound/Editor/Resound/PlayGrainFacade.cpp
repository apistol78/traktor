/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "I18N/Format.h"
#include "Sound/Resound/PlayGrainData.h"
#include "Sound/Editor/Resound/PlayGrainFacade.h"
#include "Ui/StyleBitmap.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.PlayGrainFacade", PlayGrainFacade, IGrainFacade)

PlayGrainFacade::PlayGrainFacade()
{
	m_image = new ui::StyleBitmap(L"Sound.PlayGrain");
}

ui::Widget* PlayGrainFacade::createView(IGrainData* grain, ui::Widget* parent)
{
	return nullptr;
}

ui::StyleBitmap* PlayGrainFacade::getImage(const IGrainData* grain) const
{
	return m_image;
}

std::wstring PlayGrainFacade::getText(const IGrainData* grain) const
{
	const PlayGrainData* playGrain = static_cast< const PlayGrainData* >(grain);
	return i18n::Format(
		L"RESOUND_PLAY_GRAIN_TEXT",
		playGrain->getGain().min,
		playGrain->getGain().max
	);
}

bool PlayGrainFacade::getProperties(const IGrainData* grain, std::set< std::wstring >& outProperties) const
{
	return false;
}

bool PlayGrainFacade::canHaveChildren() const
{
	return false;
}

bool PlayGrainFacade::addChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	return false;
}

bool PlayGrainFacade::removeChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	return false;
}

bool PlayGrainFacade::getChildren(IGrainData* grain, RefArray< IGrainData >& outChildren)
{
	return true;
}

}
