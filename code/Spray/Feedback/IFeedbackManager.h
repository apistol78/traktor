/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_IFeedbackManager_H
#define traktor_spray_IFeedbackManager_H

#include "Spray/Feedback/IFeedbackListener.h"

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

class T_DLLCLASS IFeedbackManager : public Object
{
	T_RTTI_CLASS;

public:
	virtual void addListener(FeedbackType type, IFeedbackListener* listener) = 0;

	virtual void removeListener(FeedbackType type, IFeedbackListener* listener) = 0;

	virtual void apply(FeedbackType type, const float* values, int32_t count) = 0;
};

	}
}

#endif	// traktor_spray_IFeedbackManager_H
