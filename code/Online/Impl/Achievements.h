#ifndef traktor_online_Achievements_H
#define traktor_online_Achievements_H

#include <map>
#include "Core/Thread/Semaphore.h"
#include "Online/IAchievements.h"

namespace traktor
{
	namespace online
	{

class IAchievementsProvider;
class TaskQueue;

class Achievements : public IAchievements
{
	T_RTTI_CLASS;

public:
	virtual bool ready() const T_OVERRIDE T_FINAL;

	virtual bool enumerate(std::set< std::wstring >& outAchievementIds) const T_OVERRIDE T_FINAL;

	virtual bool have(const std::wstring& achievementId) const T_OVERRIDE T_FINAL;

	virtual Ref< Result > set(const std::wstring& achievementId, bool reward) T_OVERRIDE T_FINAL;

private:
	friend class SessionManager;

	Ref< IAchievementsProvider > m_provider;
	Ref< TaskQueue > m_taskQueue;
	mutable Semaphore m_lock;
	std::map< std::wstring, bool > m_achievements;
	bool m_ready;

	Achievements(IAchievementsProvider* provider, TaskQueue* taskQueue);

	void enqueueEnumeration();

	void callbackEnumAchievements(const std::map< std::wstring, bool >& achievements);
};

	}
}

#endif	// traktor_online_Achievements_H
