/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_TaskEnumLeaderboards_H
#define traktor_online_TaskEnumLeaderboards_H

#include <map>
#include "Online/Impl/ITask.h"
#include "Online/Provider/ILeaderboardsProvider.h"

namespace traktor
{
	namespace online
	{

class TaskEnumLeaderboards : public ITask
{
	T_RTTI_CLASS;

public:
	typedef void (Object::*sink_method_t)(const std::map< std::wstring, ILeaderboardsProvider::LeaderboardData >&);

	TaskEnumLeaderboards(
		ILeaderboardsProvider* provider,
		Object* sinkObject,
		sink_method_t sinkMethod
	);

	virtual void execute(TaskQueue* taskQueue) T_OVERRIDE T_FINAL;

private:
	Ref< ILeaderboardsProvider > m_provider;
	Ref< Object > m_sinkObject;
	sink_method_t m_sinkMethod;
};

	}
}

#endif	// traktor_online_TaskEnumLeaderboards_H
