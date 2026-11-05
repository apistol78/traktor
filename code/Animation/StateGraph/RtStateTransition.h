/*
 * TRAKTOR
 * Copyright (c) 2025-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/Types.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

class RtStateNode;

/*!
 * \ingroup Animation
 */
class T_DLLCLASS RtStateTransition : public Object
{
	T_RTTI_CLASS;

public:
	RtStateTransition() = default;

	explicit RtStateTransition(RtStateNode* from, RtStateNode* to);

	RtStateNode* getFrom() const { return m_from; }

	RtStateNode* getTo() const { return m_to; }

	Moment getMoment() const { return m_moment; }

	float getDuration() const { return m_duration; }

	bool haveConditions() const { return !m_conditions.empty(); }

	bool conditionSatisfied(const AlignedVector< bool >& values) const;

private:
	friend class RtStateTransitionData;

	struct Condition
	{
		int32_t parameter = -1;
		bool inverted = false;
	};

	Ref< RtStateNode > m_from;
	Ref< RtStateNode > m_to;
	Moment m_moment = Moment::End;
	float m_duration = 0.0f;
	AlignedVector< Condition > m_conditions;
};

}
