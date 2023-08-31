/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "I18N/Format.h"
#include "Sound/Resound/EnvelopeGrainData.h"
#include "Sound/Editor/Resound/EnvelopeGrainFacade.h"
#include "Ui/Event.h"
#include "Ui/Envelope/DefaultEnvelopeEvaluator.h"
#include "Ui/Envelope/EnvelopeContentChangeEvent.h"
#include "Ui/Envelope/EnvelopeControl.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const Color4ub c_rangeColors[] =
{
	Color4ub(255, 200, 200, 100),
	Color4ub(200, 255, 200, 100),
	Color4ub(200, 200, 255, 100),
	Color4ub(255, 255, 200, 100),
	Color4ub(255, 200, 255, 100),
	Color4ub(200, 255, 255, 100)
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.EnvelopeGrainFacade", EnvelopeGrainFacade, IGrainFacade)

ui::Widget* EnvelopeGrainFacade::createView(IGrainData* grain, ui::Widget* parent)
{
	EnvelopeGrainData* envelopeGrain = checked_type_cast< EnvelopeGrainData*, false >(grain);
	const float* levels = envelopeGrain->getLevels();

	Ref< ui::EnvelopeControl > envelopeControl = new ui::EnvelopeControl();
	envelopeControl->create(parent, new ui::DefaultEnvelopeEvaluator< HermiteEvaluator >(), 0.0f, 1.0f, ui::WsDoubleBuffer);

	envelopeControl->insertKey(new ui::EnvelopeKey(0.0f, levels[0], true));
	envelopeControl->insertKey(new ui::EnvelopeKey(envelopeGrain->getMid(), levels[1], false));
	envelopeControl->insertKey(new ui::EnvelopeKey(1.0f, levels[2], true));

	envelopeControl->setData(L"GRAIN", grain);
	envelopeControl->addEventHandler< ui::EnvelopeContentChangeEvent >(this, &EnvelopeGrainFacade::eventEnvelopeChange);

	return envelopeControl;
}

int32_t EnvelopeGrainFacade::getImage(const IGrainData* grain) const
{
	return 5;
}

std::wstring EnvelopeGrainFacade::getText(const IGrainData* grain) const
{
	int32_t count = (int32_t)checked_type_cast< const EnvelopeGrainData* >(grain)->getGrains().size();
	return i18n::Format(L"RESOUND_ENVELOPE_GRAIN_TEXT", int32_t(count));
}

bool EnvelopeGrainFacade::getProperties(const IGrainData* grain, std::set< std::wstring >& outProperties) const
{
	const EnvelopeGrainData* envelopeGrain = checked_type_cast< const EnvelopeGrainData*, false >(grain);
	outProperties.insert(envelopeGrain->getId());
	return true;
}

bool EnvelopeGrainFacade::canHaveChildren() const
{
	return true;
}

bool EnvelopeGrainFacade::addChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	checked_type_cast< EnvelopeGrainData*, false >(parentGrain)->addGrain(childGrain, 0.0f, 1.0f, 0.2f, 0.2f);
	return true;
}

bool EnvelopeGrainFacade::removeChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	checked_type_cast< EnvelopeGrainData*, false >(parentGrain)->removeGrain(childGrain);
	return true;
}

bool EnvelopeGrainFacade::getChildren(IGrainData* grain, RefArray< IGrainData >& outChildren)
{
	const std::vector< EnvelopeGrainData::GrainData >& grains = checked_type_cast< EnvelopeGrainData*, false >(grain)->getGrains();
	for (std::vector< EnvelopeGrainData::GrainData >::const_iterator i = grains.begin(); i != grains.end(); ++i)
		outChildren.push_back(i->grain);
	return true;
}

void EnvelopeGrainFacade::eventEnvelopeChange(ui::EnvelopeContentChangeEvent* event)
{
	ui::EnvelopeControl* envelopeControl = checked_type_cast< ui::EnvelopeControl* >(event->getSender());

	EnvelopeGrainData* envelopeGrain = envelopeControl->getData< EnvelopeGrainData >(L"GRAIN");
	T_ASSERT(envelopeGrain);

	const RefArray< ui::EnvelopeKey >& keys = envelopeControl->getKeys();
	T_ASSERT(keys.size() == 3);

	float levels[] =
	{
		keys[0]->getValue(),
		keys[1]->getValue(),
		keys[2]->getValue()
	};
	envelopeGrain->setLevels(levels);
	envelopeGrain->setMid(keys[1]->getT());
}

	}
}
