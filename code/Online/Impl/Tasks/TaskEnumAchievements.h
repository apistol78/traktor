/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_TaskEnumAchievements_H
#define traktor_online_TaskEnumAchievements_H

#include <map>
#include "Online/Impl/ITask.h"

namespace traktor
{
	namespace online
	{

class IAchievementsProvider;

class TaskEnumAchievements : public ITask
{
	T_RTTI_CLASS;

public:
	typedef void (Object::*sink_method_t)(const std::map< std::wstring, bool >&);

	TaskEnumAchievements(
		IAchievementsProvider* provider,
		Object* sinkObject,
		sink_method_t sinkMethod
	);

	virtual void execute(TaskQueue* taskQueue) T_OVERRIDE T_FINAL;

private:
	Ref< IAchievementsProvider > m_provider;
	Ref< Object > m_sinkObject;
	sink_method_t m_sinkMethod;
};

	}
}

#endif	// traktor_online_TaskEnumAchievements_H
