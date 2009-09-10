#ifndef traktor_online_IUser_H
#define traktor_online_IUser_H

#include "Core/Object.h"
#include "Core/Heap/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

class IAchievement;

class T_DLLCLASS IUser : public Object
{
	T_RTTI_CLASS(IUser)

public:
	virtual std::wstring getName() const = 0;

	virtual bool getFriends(RefArray< IUser >& outFriends) const = 0;

	virtual bool getAchievements(RefArray< IAchievement >& outAchievements) const = 0;

	virtual bool rewardAchievement(IAchievement* achievement) = 0;

	virtual bool sendMessage(const std::wstring& message) const = 0;
};

	}
}

#endif	// traktor_online_IUser_H
