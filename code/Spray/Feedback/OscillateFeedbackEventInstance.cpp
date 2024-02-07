/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Math/Random.h"
#include "Spray/Feedback/OscillateFeedbackEventData.h"
#include "Spray/Feedback/OscillateFeedbackEventInstance.h"
#include "Spray/Feedback/IFeedbackManager.h"

namespace traktor::spray
{
	namespace
	{

Random s_random;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.OscillateFeedbackEventInstance", OscillateFeedbackEventInstance, world::IEntityEventInstance)

OscillateFeedbackEventInstance::OscillateFeedbackEventInstance(const OscillateFeedbackEventData* data, IFeedbackManager* feedbackManager)
:	m_data(data)
,	m_feedbackManager(feedbackManager)
,	m_time(0.0f)
{
}

bool OscillateFeedbackEventInstance::update(const world::UpdateParams& update)
{
	bool finished = true;
	float values[4];

	for (int32_t i = 0; i < 4; ++i)
	{
		const OscillateFeedbackEventData::OscillatingValue& ov = m_data->getValue(i);
		if (m_time < ov.duration)
		{
			const float f = m_time / ov.duration;
			const float m = (1.0f - f * f * f) * ov.magnitude;
			const float v = lerp(std::sin(ov.frequency * f * PI), s_random.nextFloat() * 2.0f - 1.0f, ov.noise) * m;

			values[i] = v;
			finished = false;
		}
		else
			values[i] = 0.0f;
	}

	if (m_feedbackManager)
		m_feedbackManager->apply(m_data->getType(), values, sizeof_array(values));

	m_time += update.deltaTime;
	return !finished;
}

void OscillateFeedbackEventInstance::cancel(world::Cancel when)
{
}

}
