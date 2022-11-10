/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Thread/Semaphore.h"
#include "Runtime/IStateManager.h"

namespace traktor
{
	namespace runtime
	{

/*! State manager.
 * \ingroup Runtime
 */
class StateManager : public IStateManager
{
	T_RTTI_CLASS;

public:
	/*! Destroy state manager. */
	void destroy();

	/*! Enter state transition.
	 *
	 * \param state New state.
	 */
	virtual void enter(IState* state) override final;

	/*! Get current state.
	 *
	 * \return Current state.
	 */
	IState* getCurrent() { return m_current; }

	/*! Get next state.
	 *
	 * \return Next state.
	 */
	IState* getNext() { return m_next; }

private:
	friend class Application;

	Semaphore m_lock;
	Ref< IState > m_current;
	Ref< IState > m_next;

	/*! Begin update transition.
	 *
	 * \return True if transition needs to be performed.
	 */
	bool beginTransition();

	/*! Leave current state.
	 */
	void leaveCurrent();

	/*! Enter next state.
	 */
	void enterNext();
};

	}
}

