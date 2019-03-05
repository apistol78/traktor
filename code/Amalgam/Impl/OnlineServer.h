#pragma once

#include "Amalgam/IOnlineServer.h"
#include "Core/Ref.h"

namespace traktor
{

class PropertyGroup;

	namespace db
	{

class Database;

	}

	namespace amalgam
	{

class IAudioServer;

/*! \brief
 * \ingroup Amalgam
 */
class OnlineServer : public IOnlineServer
{
	T_RTTI_CLASS;

public:
	bool create(const PropertyGroup* defaultSettings, PropertyGroup* settings, db::Database* db);

	void destroy();

	void setupVoice(IAudioServer* audioServer);

	int32_t reconfigure(const PropertyGroup* settings);

	virtual online::ISessionManager* getSessionManager() override final;

private:
	Ref< online::ISessionManager > m_sessionManager;
};

	}
}

