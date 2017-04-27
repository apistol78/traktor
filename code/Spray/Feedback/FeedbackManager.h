/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_FeedbackManager_H
#define traktor_spray_FeedbackManager_H

#include <map>
#include <vector>
#include "Spray/Feedback/IFeedbackManager.h"

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

class T_DLLCLASS FeedbackManager : public IFeedbackManager
{
	T_RTTI_CLASS;

public:
	virtual void addListener(FeedbackType type, IFeedbackListener* listener) T_OVERRIDE T_FINAL;

	virtual void removeListener(FeedbackType type, IFeedbackListener* listener) T_OVERRIDE T_FINAL;

	virtual void apply(FeedbackType type, const float* values, int32_t count) T_OVERRIDE T_FINAL;

private:
	std::map< FeedbackType, std::vector< IFeedbackListener* > > m_listeners;
};

	}
}

#endif	// traktor_spray_FeedbackManager_H
