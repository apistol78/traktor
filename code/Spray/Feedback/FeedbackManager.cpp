#include <algorithm>
#include "Spray/Feedback/FeedbackManager.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.FeedbackManager", FeedbackManager, IFeedbackManager)

void FeedbackManager::addListener(FeedbackType type, IFeedbackListener* listener)
{
	m_listeners[type].push_back(listener);
}

void FeedbackManager::removeListener(FeedbackType type, IFeedbackListener* listener)
{
	std::vector< IFeedbackListener* >& listeners = m_listeners[type];
	std::vector< IFeedbackListener* >::iterator it = std::find(listeners.begin(), listeners.end(), listener);
	if (it != listeners.end())
		listeners.erase(it);
}

void FeedbackManager::apply(FeedbackType type, const float* values, int32_t count)
{
	std::vector< IFeedbackListener* >& listeners = m_listeners[type];
	for (std::vector< IFeedbackListener* >::iterator i = listeners.begin(); i != listeners.end(); ++i)
		(*i)->feedbackValues(type, values, count);
}

	}
}
