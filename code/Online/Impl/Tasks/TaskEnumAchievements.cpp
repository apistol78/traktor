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
	log::debug << L"Online; Begin enumerating achievements" << Endl;
	std::map< std::wstring, bool > achievements;
	m_provider->enumerate(achievements);
	(m_sinkObject->*m_sinkMethod)(achievements);
	log::debug << L"Online; Finished enumerating achievements" << Endl;
}

	}
}
