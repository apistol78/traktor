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

Ref< world::IEntityEventInstance > OscillateFeedbackEvent::createInstance(world::IEntityEventManager* eventManager, world::Entity* sender, const Transform& Toffset) const
{
	return new OscillateFeedbackEventInstance(m_data, m_feedbackManager);
}

	}
}
