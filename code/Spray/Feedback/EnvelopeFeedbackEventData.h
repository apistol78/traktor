/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_EnvelopeFeedbackEventData_H
#define traktor_spray_EnvelopeFeedbackEventData_H

#include <vector>
#include "Spray/Feedback/IFeedbackListener.h"
#include "World/IEntityEventData.h"

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

class T_DLLCLASS EnvelopeFeedbackEventData : public world::IEntityEventData
{
	T_RTTI_CLASS;

public:
	struct TimedValue
	{
		float at;
		float value;

		TimedValue();

		void serialize(ISerializer& s);
	};

	EnvelopeFeedbackEventData();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	FeedbackType getType() const { return m_type; }

	const std::vector< TimedValue >& getEnvelope(int32_t index) const { return m_envelopes[index]; }

private:
	FeedbackType m_type;
	std::vector< TimedValue > m_envelopes[4];
};

	}
}

#endif	// traktor_spray_EnvelopeFeedbackEventData_H
