#ifndef traktor_online_SessionLocal_H
#define traktor_online_SessionLocal_H

#include <sysutil/sysutil_savedata.h>
#include "Online/ISession.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_PSN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

class UserPsn;

class T_DLLCLASS SessionPsn : public ISession
{
	T_RTTI_CLASS;

public:
	SessionPsn(UserPsn* user);

	bool create();

	virtual void destroy();

	virtual bool isConnected() const;

	virtual Ref< IUser > getUser();

	virtual bool rewardAchievement(const std::wstring& achievementId);

	virtual bool withdrawAchievement(const std::wstring& achievementId);

	virtual bool haveAchievement(const std::wstring& achievementId);

	virtual Ref< ILeaderboard > getLeaderboard(const std::wstring& id);

	virtual bool setStatValue(const std::wstring& statId, float value);

	virtual bool getStatValue(const std::wstring& statId, float& outValue);

	virtual Ref< ISaveGame > createSaveGame(const std::wstring& name, ISerializable* attachment);

	virtual bool getAvailableSaveGames(RefArray< ISaveGame >& outSaveGames) const;

	void update();

private:
	Ref< UserPsn > m_user;
	bool m_connected;
	int32_t m_titleContextId;
	std::vector< uint8_t > m_saveBuffer;
	bool m_saveBufferPending;
	mutable RefArray< ISaveGame > m_saveGames;

	static void loadFixedCallback(CellSaveDataCBResult* cbResult, CellSaveDataListGet* get, CellSaveDataFixedSet* set);

	static void loadStatCallback(CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set);

	static void loadFileCallback(CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set);

	static void saveStatCallback(CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set);

	static void saveFileCallback(CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set);
};

	}
}

#endif	// traktor_online_SessionLocal_H
