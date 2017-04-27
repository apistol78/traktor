/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Online/Impl/Tasks/TaskEnumAchievements.h"
#include "Online/Provider/IAchievementsProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskEnumAchievements", TaskEnumAchievements, ITask)

TaskEnumAchievements::TaskEnumAchievements(
	IAchievementsProvider* provider,
	Object* sinkObject,
	sink_method_t sinkMethod
)
:	m_provider(provider)
,	m_sinkObject(sinkObject)
,	m_sinkMethod(sinkMethod)
{
}

void TaskEnumAchievements::execute(TaskQueue* taskQueue)
{
	T_ASSERT (m_provider);
	T_DEBUG(L"Online; Begin enumerating achievements");
	std::map< std::wstring, bool > achievements;
	m_provider->enumerate(achievements);
	(m_sinkObject->*m_sinkMethod)(achievements);
	T_DEBUG(L"Online; Finished enumerating achievements");
}

	}
}
