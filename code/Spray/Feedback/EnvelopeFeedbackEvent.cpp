#include "Spray/Feedback/EnvelopeFeedbackEvent.h"
#include "Spray/Feedback/EnvelopeFeedbackEventInstance.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EnvelopeFeedbackEvent", EnvelopeFeedbackEvent, world::IEntityEvent)

EnvelopeFeedbackEvent::EnvelopeFeedbackEvent(const EnvelopeFeedbackEventData* data, IFeedbackManager* feedbackManager)
:	m_data(data)
,	m_feedbackManager(feedbackManager)
{
}

Ref< world::IEntityEventInstance > EnvelopeFeedbackEvent::createInstance(world::IEntityEventManager* eventManager, world::Entity* sender, const Transform& Toffset) const
{
	return new EnvelopeFeedbackEventInstance(m_data, m_feedbackManager);
}

	}
}
