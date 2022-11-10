/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Input/Binding/IInputNode.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class InputStateData;
class InputValueSet;

/*! \brief
 * \ingroup Input
 */
class T_DLLCLASS InputState : public Object
{
	T_RTTI_CLASS;

public:
	InputState();

	bool create(const InputStateData* data);

	void update(const InputValueSet& valueSet, float T, float dT);

	void reset();

	float getValue() const { return m_currentValue; }

	float getPreviousValue() const { return m_previousValue; }

	bool isDown() const;

	bool isUp() const;

	bool isPressed() const;

	bool isReleased() const;

	bool hasChanged() const;

private:
	Ref< const InputStateData > m_data;
	Ref< IInputNode::Instance > m_instance;
	bool m_active;
	float m_previousValue;
	float m_currentValue;
};

	}
}

