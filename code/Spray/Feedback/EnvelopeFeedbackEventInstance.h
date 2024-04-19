/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "World/IEntityEventInstance.h"

namespace traktor::spray
{

class EnvelopeFeedbackEventData;

class EnvelopeFeedbackEventInstance : public world::IEntityEventInstance
{
	T_RTTI_CLASS;

public:
	explicit EnvelopeFeedbackEventInstance(const EnvelopeFeedbackEventData* data);

	virtual bool update(world::World* world, const world::UpdateParams& update) override final;

	virtual void cancel(world::Cancel when) override final;

private:
	const EnvelopeFeedbackEventData* m_data;
	float m_time;
};

}
