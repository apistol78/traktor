#ifndef traktor_online_ISessionManager_H
#define traktor_online_ISessionManager_H

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

class IAchievements;
class ILeaderboards;
class ISaveData;
class IStatistics;

class T_DLLCLASS ISessionManager : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	virtual bool update() = 0;

	virtual std::wstring getLanguageCode() const = 0;

	virtual bool isConnected() const = 0;

	virtual bool requireUserAttention() const = 0;

	virtual Ref< IAchievements > getAchievements() const = 0;

	virtual Ref< ILeaderboards > getLeaderboards() const = 0;

	virtual Ref< ISaveData > getSaveData() const = 0;

	virtual Ref< IStatistics > getStatistics() const = 0;
};

	}
}

#endif	// traktor_online_ISessionManager_H
