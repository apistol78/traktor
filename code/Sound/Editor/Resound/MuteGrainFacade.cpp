/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "I18N/Format.h"
#include "Sound/Resound/MuteGrainData.h"
#include "Sound/Editor/Resound/MuteGrainFacade.h"
#include "Ui/StyleBitmap.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.MuteGrainFacade", MuteGrainFacade, IGrainFacade)

MuteGrainFacade::MuteGrainFacade()
{
	m_image = new ui::StyleBitmap(L"Sound.MuteGrain");
}

ui::Widget* MuteGrainFacade::createView(IGrainData* grain, ui::Widget* parent)
{
	return nullptr;
}

ui::StyleBitmap* MuteGrainFacade::getImage(const IGrainData* grain) const
{
	return m_image;
}

std::wstring MuteGrainFacade::getText(const IGrainData* grain) const
{
	const MuteGrainData* muteGrain = static_cast< const MuteGrainData* >(grain);
	return i18n::Format(L"RESOUND_MUTE_GRAIN_TEXT", int32_t(muteGrain->getDuration().min * 1000), int32_t(muteGrain->getDuration().max * 1000));
}

bool MuteGrainFacade::getProperties(const IGrainData* grain, std::set< std::wstring >& outProperties) const
{
	return false;
}

bool MuteGrainFacade::canHaveChildren() const
{
	return false;
}

bool MuteGrainFacade::addChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	return false;
}

bool MuteGrainFacade::removeChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	return false;
}

bool MuteGrainFacade::getChildren(IGrainData* grain, RefArray< IGrainData >& outChildren)
{
	return true;
}

}
