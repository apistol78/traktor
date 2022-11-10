/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Spark/Event.h"

namespace traktor
{
	namespace spark
	{

/*! Mouse class.
 * \ingroup Spark
 */
class Mouse : public Object
{
	T_RTTI_CLASS;

public:
	void eventMouseDown(int32_t x, int32_t y, int32_t button);

	void eventMouseUp(int32_t x, int32_t y, int32_t button);

	void eventMouseMove(int32_t x, int32_t y, int32_t button);

	void eventMouseWheel(int32_t x, int32_t y, int32_t delta);

	/*! \group Events */
	//@{

	Event* getEventButtonDown() { return &m_eventButtonDown; }

	Event* getEventButtonUp() { return &m_eventButtonUp; }

	Event* getEventMouseMove() { return &m_eventMouseMove; }

	Event* getEventMouseWheel() { return &m_eventMouseWheel; }

	//@}

private:
	Event m_eventButtonDown;
	Event m_eventButtonUp;
	Event m_eventMouseMove;
	Event m_eventMouseWheel;
};

	}
}

