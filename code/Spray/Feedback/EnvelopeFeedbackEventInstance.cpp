/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Hermite.h"
#include "Spray/Feedback/EnvelopeFeedbackEventData.h"
#include "Spray/Feedback/EnvelopeFeedbackEventInstance.h"
#include "Spray/Feedback/FeedbackComponent.h"
#include "World/World.h"

namespace traktor::spray
{
	namespace
	{

struct TimedValueAccessor
{
	static inline float time(const EnvelopeFeedbackEventData::TimedValue* keys, size_t nkeys, const EnvelopeFeedbackEventData::TimedValue& key)
	{
		return key.at;
	}

	static inline float value(const EnvelopeFeedbackEventData::TimedValue& key)
	{
		return key.value;
	}

	static inline float combine(
		float t,
		float v0, float w0,
		float v1, float w1,
		float v2, float w2,
		float v3, float w3
	)
	{
		return v0 * w0 + v1 * w1 + v2 * w2 + v3 * w3;
	}
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EnvelopeFeedbackEventInstance", EnvelopeFeedbackEventInstance, world::IEntityEventInstance)

EnvelopeFeedbackEventInstance::EnvelopeFeedbackEventInstance(const EnvelopeFeedbackEventData* data)
:	m_data(data)
,	m_time(0.0f)
{
}

bool EnvelopeFeedbackEventInstance::update(world::World* world, const world::UpdateParams& update)
{
	float duration = 0.0f;
	float values[4];

	for (int32_t i = 0; i < 4; ++i)
	{
		const std::vector< EnvelopeFeedbackEventData::TimedValue >& envelope = m_data->getEnvelope(i);
		if (!envelope.empty())
		{
			values[i] = Hermite< EnvelopeFeedbackEventData::TimedValue, float, TimedValueAccessor >(&envelope[0], envelope.size()).evaluate(m_time);
			duration = max(duration, envelope.back().at);
		}
		else
			values[i] = 0.0f;
	}

	Ref< FeedbackComponent > feedback = world->getComponent< FeedbackComponent >();
	if (feedback)
		feedback->apply(m_data->getType(), values, sizeof_array(values));

	m_time += (float)update.deltaTime;
	return m_time < duration;
}

void EnvelopeFeedbackEventInstance::cancel(world::Cancel when)
{
}

}
