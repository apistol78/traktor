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

	virtual bool update(const world::UpdateParams& update) T_OVERRIDE T_FINAL;

	virtual void build(world::IWorldRenderer* worldRenderer) T_OVERRIDE T_FINAL;

	virtual void cancel(world::CancelType when) T_OVERRIDE T_FINAL;

private:
	const EnvelopeFeedbackEventData* m_data;
	IFeedbackManager* m_feedbackManager;
	float m_time;
};

	}
}

#endif	// traktor_spray_EnvelopeFeedbackEventInstance_H
