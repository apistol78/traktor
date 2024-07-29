/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Thread/Result.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::online
{

class ILobby;

class T_DLLCLASS LobbyResult : public Result
{
	T_RTTI_CLASS;

public:
	void succeed(ILobby* lobby);

	ILobby* get() const;

private:
	Ref< ILobby > m_lobby;
};

}
