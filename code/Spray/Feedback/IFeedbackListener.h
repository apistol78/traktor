#ifndef traktor_spray_IFeedbackListener_H
#define traktor_spray_IFeedbackListener_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

enum FeedbackType
{
	FbtNone			= 0,
	FbtCamera		= 1,
	FbtPostProcess	= 2,
	FbtUI			= 3
};

class T_DLLCLASS IFeedbackListener
{
public:
	virtual ~IFeedbackListener() {}

	virtual void feedbackValues(FeedbackType type, const float* values, int32_t count) = 0;
};

	}
}

#endif	// traktor_spray_IFeedbackListener_H
