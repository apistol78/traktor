/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "World/IEntityEvent.h"

namespace traktor::spray
{

class EnvelopeFeedbackEventData;

class EnvelopeFeedbackEvent : public world::IEntityEvent
{
	T_RTTI_CLASS;

public:
	explicit EnvelopeFeedbackEvent(const EnvelopeFeedbackEventData* data);

	virtual Ref< world::IEntityEventInstance > createInstance(world::EventManagerComponent* eventManager, world::Entity* sender, const Transform& Toffset) const override final;

private:
	Ref< const EnvelopeFeedbackEventData > m_data;
};

}

