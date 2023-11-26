/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "I18N/Format.h"
#include "Sound/Resound/SequenceGrainData.h"
#include "Sound/Editor/Resound/SequenceGrainFacade.h"
#include "Ui/StyleBitmap.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SequenceGrainFacade", SequenceGrainFacade, IGrainFacade)

SequenceGrainFacade::SequenceGrainFacade()
{
	m_image = new ui::StyleBitmap(L"Sound.SequenceGrain");
}

ui::Widget* SequenceGrainFacade::createView(IGrainData* grain, ui::Widget* parent)
{
	return nullptr;
}

ui::StyleBitmap* SequenceGrainFacade::getImage(const IGrainData* grain) const
{
	return m_image;
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
	mandatory_non_null_type_cast< SequenceGrainData* >(parentGrain)->addGrain(childGrain);
	return true;
}

bool SequenceGrainFacade::removeChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	mandatory_non_null_type_cast< SequenceGrainData* >(parentGrain)->removeGrain(childGrain);
	return true;
}

bool SequenceGrainFacade::getChildren(IGrainData* grain, RefArray< IGrainData >& outChildren)
{
	outChildren = mandatory_non_null_type_cast< SequenceGrainData* >(grain)->getGrains();
	return true;
}

}
