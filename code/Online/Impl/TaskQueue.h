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
#include "Core/RefArray.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Signal.h"

namespace traktor
{

class Thread;

	namespace online
	{

class ISaveGame;
class ITask;

class TaskQueue : public Object
{
	T_RTTI_CLASS;

public:
	bool create();

	void destroy();

	bool add(ITask* task);

private:
	Thread* m_thread;
	Semaphore m_queueLock;
	Signal m_queuedSignal;
	RefArray< ITask > m_queue;

	void flush();

	void threadQueue();
};

	}
}

