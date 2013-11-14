#include "Core/Math/Const.h"
#include "Spray/Feedback/OscillateFeedbackEventData.h"
#include "Spray/Feedback/OscillateFeedbackEventInstance.h"
#include "Spray/Feedback/IFeedbackManager.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.OscillateFeedbackEventInstance", OscillateFeedbackEventInstance, world::IEntityEventInstance)

OscillateFeedbackEventInstance::OscillateFeedbackEventInstance(const OscillateFeedbackEventData* data, IFeedbackManager* feedbackManager)
:	m_data(data)
,	m_feedbackManager(feedbackManager)
,	m_time(0.0f)
{
}

bool OscillateFeedbackEventInstance::update(const world::UpdateParams& update)
{
	float values[4];
	for (int32_t i = 0; i < 4; ++i)
	{
		const OscillateFeedbackEventData::OscillatingValue& ov = m_data->getValue(i);
		if (m_time < ov.duration)
		{
			float f = m_time / ov.duration;
			float v = std::sin(ov.frequency * f * PI) * (1.0f - f * f * f) * ov.magnitude;
			values[i] = v;
		}
		else
			values[i] = 0.0f;
	}

	if (m_feedbackManager)
		m_feedbackManager->apply(m_data->getType(), values, sizeof_array(values));

	m_time += update.deltaTime;
	return true;
}

void OscillateFeedbackEventInstance::build(world::IWorldRenderer* worldRenderer)
{
}

void OscillateFeedbackEventInstance::cancel()
{
}

	}
}
