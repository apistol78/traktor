/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Online/Impl/ITask.h"

namespace traktor
{
	namespace online
	{

class ISessionManagerProvider;

class TaskUpdateSessionManager : public ITask
{
	T_RTTI_CLASS;

public:
	TaskUpdateSessionManager(ISessionManagerProvider* provider);

	virtual void execute(TaskQueue* taskQueue) override final;

	bool completed() const { return m_completed; }

	void reset() { m_completed = false; }

private:
	Ref< ISessionManagerProvider > m_provider;
	bool m_completed;
};

	}
}

