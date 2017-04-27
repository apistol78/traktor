/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_OnlineServer_H
#define traktor_amalgam_OnlineServer_H

#include "Amalgam/Game/IOnlineServer.h"

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

	virtual online::ISessionManager* getSessionManager() T_OVERRIDE T_FINAL;

private:
	Ref< online::ISessionManager > m_sessionManager;
};

	}
}

#endif	// traktor_amalgam_OnlineServer_H
