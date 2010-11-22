#ifndef traktor_amalgam_OnlineServer_H
#define traktor_amalgam_OnlineServer_H

#include "Amalgam/IOnlineServer.h"

namespace traktor
{

class Settings;

	namespace online
	{

class ISessionManagerProvider;

	}

	namespace amalgam
	{

class OnlineServer : public IOnlineServer
{
	T_RTTI_CLASS;

public:
	bool create(online::ISessionManagerProvider* provider);

	void destroy();

	int32_t reconfigure(const Settings* settings);

	virtual online::ISessionManager* getSessionManager();

private:
	Ref< online::ISessionManager > m_sessionManager;
};

	}
}

#endif	// traktor_amalgam_OnlineServer_H
