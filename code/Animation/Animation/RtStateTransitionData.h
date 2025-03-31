/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/Types.h"
#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

class RtStateData;
class RtStateGraph;
class RtStateTransition;

/*!
 * \ingroup Animation
 */
class T_DLLCLASS RtStateTransitionData : public ISerializable
{
	T_RTTI_CLASS;

public:
	Ref< RtStateTransition > createInstance(RtStateGraph* stateGraph) const;

	virtual void serialize(ISerializer& s);

private:
	friend class StateGraphCompiler;

	struct Condition
	{
		std::wstring name;
		bool inverted = false;

		void serialize(ISerializer& s);
	};

	int32_t m_from = -1;
	int32_t m_to = -1;
	Moment m_moment = Moment::End;
	float m_duration = 0.0f;
	AlignedVector< Condition > m_conditions;
};

}
