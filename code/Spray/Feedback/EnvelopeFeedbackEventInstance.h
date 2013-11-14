#ifndef traktor_spray_EnvelopeFeedbackEventInstance_H
#define traktor_spray_EnvelopeFeedbackEventInstance_H

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

	virtual bool update(const world::UpdateParams& update);

	virtual void build(world::IWorldRenderer* worldRenderer);

	virtual void cancel();

private:
	const EnvelopeFeedbackEventData* m_data;
	IFeedbackManager* m_feedbackManager;
	float m_time;
};

	}
}

#endif	// traktor_spray_EnvelopeFeedbackEventInstance_H
