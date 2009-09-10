#ifndef traktor_online_ISession_H
#define traktor_online_ISession_H

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

class Serializable;

	namespace online
	{

class IUser;
class IAchievement;
class ISaveGame;

class T_DLLCLASS ISession : public Object
{
	T_RTTI_CLASS(ISession)

public:
	virtual void destroy() = 0;

	virtual bool isConnected() const = 0;

	virtual IUser* getUser() = 0;

	virtual bool getAvailableAchievements(RefArray< IAchievement >& outAchievements) const = 0;

	virtual ISaveGame* createSaveGame(const std::wstring& name, Serializable* attachment) = 0;

	virtual bool getAvailableSaveGames(RefArray< ISaveGame >& outSaveGames) const = 0;
};

	}
}

#endif	// traktor_online_ISession_H
