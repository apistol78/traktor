#include "Amalgam/IAudioServer.h"
#include "Amalgam/Types.h"
#include "Amalgam/Impl/LibraryHelper.h"
#include "Amalgam/Impl/OnlineServer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Online/IGameConfiguration.h"
#include "Online/IVoiceChat.h"
#include "Online/Impl/SessionManager.h"
#include "Online/Provider/ISessionManagerProvider.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.OnlineServer", OnlineServer, IOnlineServer)

bool OnlineServer::create(const PropertyGroup* defaultSettings, const PropertyGroup* settings, db::Database* db)
{
	Guid configGuid(defaultSettings->getProperty< PropertyString >(L"Online.Config"));
	if (!configGuid.isValid() || configGuid.isNull())
	{
		log::error << L"Online server failed; invalid game guid" << Endl;
		return false;
	}

	Ref< online::IGameConfiguration > gameConfiguration = db->getObjectReadOnly< online::IGameConfiguration >(configGuid);
	if (!gameConfiguration)
	{
		log::error << L"Online server failed; no such game configuration" << Endl;
		return false;
	}

	std::wstring providerType = defaultSettings->getProperty< PropertyString >(L"Online.Type");

	Ref< online::ISessionManagerProvider > sessionManagerProvider = loadAndInstantiate< online::ISessionManagerProvider >(providerType);
	if (!sessionManagerProvider)
	{
		log::error << L"Online server failed; no such type \"" << providerType << L"\"" << Endl;
		return false;
	}

	bool downloadableContent = defaultSettings->getProperty< PropertyBoolean >(L"Online.DownloadableContent", true);

	Ref< online::SessionManager > sessionManager = new online::SessionManager();
	if (!sessionManager->create(sessionManagerProvider, gameConfiguration, downloadableContent))
	{
		log::error << L"Online server failed; unable to create session manager" << Endl;
		return false;
	}

	m_sessionManager = sessionManager;
	return true;
}

void OnlineServer::destroy()
{
	safeDestroy(m_sessionManager);
}

void OnlineServer::setupVoice(IAudioServer* audioServer)
{
	if (m_sessionManager->getVoiceChat())
		m_sessionManager->getVoiceChat()->attachSoundPlayer(audioServer->getSoundPlayer());
}

int32_t OnlineServer::reconfigure(const PropertyGroup* settings)
{
	return CrUnaffected;
}

online::ISessionManager* OnlineServer::getSessionManager()
{
	return m_sessionManager;
}

	}
}
