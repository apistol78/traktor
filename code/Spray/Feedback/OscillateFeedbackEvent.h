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

namespace traktor
{
	namespace spray
	{

class OscillateFeedbackEventData;
class IFeedbackManager;

class OscillateFeedbackEvent : public world::IEntityEvent
{
	T_RTTI_CLASS;

public:
	OscillateFeedbackEvent(const OscillateFeedbackEventData* data, IFeedbackManager* feedbackManager);

	virtual Ref< world::IEntityEventInstance > createInstance(world::EntityEventManager* eventManager, world::Entity* sender, const Transform& Toffset) const override final;

private:
	Ref< const OscillateFeedbackEventData > m_data;
	IFeedbackManager* m_feedbackManager;
};

	}
}

