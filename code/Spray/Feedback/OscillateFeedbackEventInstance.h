#pragma once

#include "World/IEntityEventInstance.h"

namespace traktor
{
	namespace spray
	{

class OscillateFeedbackEventData;
class IFeedbackManager;

class OscillateFeedbackEventInstance : public world::IEntityEventInstance
{
	T_RTTI_CLASS;

public:
	OscillateFeedbackEventInstance(const OscillateFeedbackEventData* data, IFeedbackManager* feedbackManager);

	virtual bool update(const world::UpdateParams& update) override final;

	virtual void attach(world::IWorldRenderer* worldRenderer) override final;

	virtual void cancel(world::CancelType when) override final;

private:
	const OscillateFeedbackEventData* m_data;
	IFeedbackManager* m_feedbackManager;
	float m_time;
};

	}
}

