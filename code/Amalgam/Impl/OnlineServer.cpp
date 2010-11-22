#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Online/Impl/SessionManager.h"
#include "Amalgam/Types.h"
#include "Amalgam/Impl/OnlineServer.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.OnlineServer", OnlineServer, IOnlineServer)

bool OnlineServer::create(online::ISessionManagerProvider* provider)
{
	if (!provider)
		return false;

	Ref< online::SessionManager > sessionManager = new online::SessionManager();
	if (!sessionManager->create(provider))
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

int32_t OnlineServer::reconfigure(const Settings* settings)
{
	return CrUnaffected;
}

online::ISessionManager* OnlineServer::getSessionManager()
{
	return m_sessionManager;
}

	}
}
