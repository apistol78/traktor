/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_TaskUpdateSessionManager_H
#define traktor_online_TaskUpdateSessionManager_H

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

	virtual void execute(TaskQueue* taskQueue) T_OVERRIDE T_FINAL;

	bool completed() const { return m_completed; }

	void reset() { m_completed = false; }

private:
	Ref< ISessionManagerProvider > m_provider;
	bool m_completed;
};

	}
}

#endif	// traktor_online_TaskUpdateSessionManager_H
