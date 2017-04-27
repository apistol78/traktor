/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_OscillateFeedbackEventData_H
#define traktor_spray_OscillateFeedbackEventData_H

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

class T_DLLCLASS OscillateFeedbackEventData : public world::IEntityEventData
{
	T_RTTI_CLASS;

public:
	struct OscillatingValue
	{
		float duration;
		int32_t frequency;
		float magnitude;
		float noise;

		OscillatingValue();

		void serialize(ISerializer& s);
	};

	OscillateFeedbackEventData();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	FeedbackType getType() const { return m_type; }

	const OscillatingValue& getValue(int32_t index) const { return m_values[index]; }

private:
	FeedbackType m_type;
	OscillatingValue m_values[4];
};

	}
}

#endif	// traktor_spray_OscillateFeedbackEventData_H
