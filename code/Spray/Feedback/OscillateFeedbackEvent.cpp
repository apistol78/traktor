/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/Feedback/OscillateFeedbackEvent.h"
#include "Spray/Feedback/OscillateFeedbackEventInstance.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.OscillateFeedbackEvent", OscillateFeedbackEvent, world::IEntityEvent)

OscillateFeedbackEvent::OscillateFeedbackEvent(const OscillateFeedbackEventData* data, IFeedbackManager* feedbackManager)
:	m_data(data)
,	m_feedbackManager(feedbackManager)
{
}

Ref< world::IEntityEventInstance > OscillateFeedbackEvent::createInstance(world::EntityEventManager* eventManager, world::Entity* sender, const Transform& Toffset) const
{
	return new OscillateFeedbackEventInstance(m_data, m_feedbackManager);
}

	}
}
