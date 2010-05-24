#ifndef traktor_online_SaveGameSteam_H
#define traktor_online_SaveGameSteam_H

#include <steam/steam_api.h>
#include "Online/ISaveGame.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_STEAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

class ISteamUser;

namespace traktor
{
	namespace online
	{

class SaveGameSteam : public ISaveGame
{
	T_RTTI_CLASS;

public:
	SaveGameSteam(const std::wstring& name);

	virtual std::wstring getName() const;

	virtual Ref< ISerializable > getAttachment() const;

private:
	std::wstring m_name;
};

	}
}

#endif	// traktor_online_SaveGameSteam_H
