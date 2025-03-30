/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

class StateNode;

/*! Animation graph transition.
 * \ingroup Animation
 */
class T_DLLCLASS StateTransition : public ISerializable
{
	T_RTTI_CLASS;

public:
	enum class Moment
	{
		Immediatly,
		End
	};

	StateTransition() = default;

	explicit StateTransition(StateNode* from, StateNode* to);

	StateNode* from() const;

	StateNode* to() const;

	Moment getMoment() const;

	float getDuration() const;

	const std::wstring& getCondition() const;

	virtual void serialize(ISerializer& s) override final;

private:
	Ref< StateNode > m_from;
	Ref< StateNode > m_to;
	Moment m_moment = Moment::End;
	float m_duration = 0.0f;
	std::wstring m_condition;
};

}
