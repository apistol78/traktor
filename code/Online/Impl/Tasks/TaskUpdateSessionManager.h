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

