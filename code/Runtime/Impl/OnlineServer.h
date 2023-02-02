/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Runtime/IOnlineServer.h"
#include "Core/Ref.h"

namespace traktor
{

class PropertyGroup;

}

namespace traktor::db
{

class Database;

}

namespace traktor::runtime
{

class IAudioServer;

/*!
 * \ingroup Runtime
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
