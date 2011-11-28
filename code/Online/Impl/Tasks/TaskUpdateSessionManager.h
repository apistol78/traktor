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

	virtual void execute(TaskQueue* taskQueue);

private:
	Ref< ISessionManagerProvider > m_provider;
};

	}
}

#endif	// traktor_online_TaskUpdateSessionManager_H
