#ifndef traktor_online_ISessionManagerProvider_H
#define traktor_online_ISessionManagerProvider_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

class IAchievementsProvider;
class ILeaderboardsProvider;
class ISaveDataProvider;
class IStatisticsProvider;

class T_DLLCLASS ISessionManagerProvider : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	virtual bool update() = 0;

	virtual std::wstring getLanguageCode() const = 0;

	virtual bool isConnected() const = 0;

	virtual bool requireUserAttention() const = 0;

	virtual Ref< IAchievementsProvider > getAchievements() const = 0;

	virtual Ref< ILeaderboardsProvider > getLeaderboards() const = 0;

	virtual Ref< ISaveDataProvider > getSaveData() const = 0;

	virtual Ref< IStatisticsProvider > getStatistics() const = 0;
};

	}
}

#endif	// traktor_online_ISessionManagerProvider_H
