#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Online/Impl/Achievements.h"
#include "Online/Impl/TaskQueue.h"
#include "Online/Impl/Tasks/TaskAchievement.h"
#include "Online/Impl/Tasks/TaskEnumAchievements.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.Achievements", Achievements, IAchievements)

bool Achievements::ready() const
{
	return m_ready;
}

bool Achievements::enumerate(std::set< std::wstring >& outAchievementIds) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (std::map< std::wstring, bool >::const_iterator i = m_achievements.begin(); i != m_achievements.end(); ++i)
		outAchievementIds.insert(i->first);
	return true;
}

bool Achievements::have(const std::wstring& achievementId) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	std::map< std::wstring, bool >::const_iterator i = m_achievements.find(achievementId);
	if (i == m_achievements.end())
	{
		log::warning << L"Achievement error; No such achievement provided, \"" << achievementId << L"\"" << Endl;
		return false;
	}

	return i->second;
}

Ref< Result > Achievements::set(const std::wstring& achievementId, bool reward)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	std::map< std::wstring, bool >::iterator i = m_achievements.find(achievementId);
	if (i == m_achievements.end())
	{
		log::warning << L"Achievement error; No such achievement provided, \"" << achievementId << L"\"" << Endl;
		return 0;
	}

	if (i->second == reward)
		return new Result(true);

	Ref< Result > result = new Result();
	if (m_taskQueue->add(new TaskAchievement(
		m_provider,
		achievementId,
		reward,
		result
	)))
	{
		i->second = reward;
		return result;
	}
	else
		return 0;
}

Achievements::Achievements(IAchievementsProvider* provider, TaskQueue* taskQueue)
:	m_provider(provider)
,	m_taskQueue(taskQueue)
,	m_ready(false)
{
}

void Achievements::enqueueEnumeration()
{
	m_taskQueue->add(new TaskEnumAchievements(
		m_provider,
		this,
		(TaskEnumAchievements::sink_method_t)&Achievements::callbackEnumAchievements
	));
}

void Achievements::callbackEnumAchievements(const std::map< std::wstring, bool >& achievements)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_achievements = achievements;
	m_ready = true;
}

	}
}
