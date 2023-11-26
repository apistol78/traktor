/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "I18N/Text.h"
#include "Sound/Resound/BlendGrainData.h"
#include "Sound/Editor/Resound/BlendGrainFacade.h"
#include "Ui/StyleBitmap.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.BlendGrainFacade", BlendGrainFacade, IGrainFacade)

BlendGrainFacade::BlendGrainFacade()
{
	m_image = new ui::StyleBitmap(L"Sound.BlendGrain");
}

ui::Widget* BlendGrainFacade::createView(IGrainData* grain, ui::Widget* parent)
{
	return nullptr;
}

ui::StyleBitmap* BlendGrainFacade::getImage(const IGrainData* grain) const
{
	return m_image;
}

std::wstring BlendGrainFacade::getText(const IGrainData* grain) const
{
	return i18n::Text(L"RESOUND_BLEND_GRAIN_TEXT");
}

bool BlendGrainFacade::getProperties(const IGrainData* grain, std::set< std::wstring >& outProperties) const
{
	const BlendGrainData* blendGrain = mandatory_non_null_type_cast< const BlendGrainData* >(grain);
	outProperties.insert(blendGrain->getId());
	return true;
}

bool BlendGrainFacade::canHaveChildren() const
{
	return false;
}

bool BlendGrainFacade::addChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	return false;
}

bool BlendGrainFacade::removeChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	return false;
}

bool BlendGrainFacade::getChildren(IGrainData* grain, RefArray< IGrainData >& outChildren)
{
	BlendGrainData* blendGrain = checked_type_cast< BlendGrainData*, false >(grain);
	outChildren.push_back(blendGrain->getGrains()[0]);
	outChildren.push_back(blendGrain->getGrains()[1]);
	return true;
}

}
