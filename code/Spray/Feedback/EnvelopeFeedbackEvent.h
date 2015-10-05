#ifndef traktor_spray_EnvelopeFeedbackEvent_H
#define traktor_spray_EnvelopeFeedbackEvent_H

#include "World/IEntityEvent.h"

namespace traktor
{
	namespace spray
	{

class EnvelopeFeedbackEventData;
class IFeedbackManager;

class EnvelopeFeedbackEvent : public world::IEntityEvent
{
	T_RTTI_CLASS;

public:
	EnvelopeFeedbackEvent(const EnvelopeFeedbackEventData* data, IFeedbackManager* feedbackManager);

	virtual Ref< world::IEntityEventInstance > createInstance(world::IEntityEventManager* eventManager, world::Entity* sender, const Transform& Toffset) const T_OVERRIDE T_FINAL;

private:
	Ref< const EnvelopeFeedbackEventData > m_data;
	IFeedbackManager* m_feedbackManager;
};

	}
}

#endif	// traktor_spray_EnvelopeFeedbackEvent_H
