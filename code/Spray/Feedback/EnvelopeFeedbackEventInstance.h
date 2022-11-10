/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "World/IEntityEventInstance.h"

namespace traktor
{
	namespace spray
	{

class EnvelopeFeedbackEventData;
class IFeedbackManager;

class EnvelopeFeedbackEventInstance : public world::IEntityEventInstance
{
	T_RTTI_CLASS;

public:
	EnvelopeFeedbackEventInstance(const EnvelopeFeedbackEventData* data, IFeedbackManager* feedbackManager);

	virtual bool update(const world::UpdateParams& update) override final;

	virtual void gather(const std::function< void(world::Entity*) >& fn) const override final;

	virtual void cancel(world::Cancel when) override final;

private:
	const EnvelopeFeedbackEventData* m_data;
	IFeedbackManager* m_feedbackManager;
	float m_time;
};

	}
}

