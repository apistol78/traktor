/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

	virtual void serialize(ISerializer& s) override final;

	FeedbackType getType() const { return m_type; }

	const std::vector< TimedValue >& getEnvelope(int32_t index) const { return m_envelopes[index]; }

private:
	FeedbackType m_type;
	std::vector< TimedValue > m_envelopes[4];
};

	}
}

