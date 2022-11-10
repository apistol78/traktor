/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Net/Network.h"
#include "Net/Platform.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace net
	{

int32_t Network::ms_initialized = 0;

bool Network::initialize()
{
	if (ms_initialized > 0)
	{
		ms_initialized++;
		return true;
	}

#if defined(_WIN32)
	static WSADATA s_wsa;
	if (WSAStartup(MAKEWORD(2, 2), &s_wsa) != 0)
	{
		log::error << L"WSAStartup failed; GetLastError = " << uint32_t(GetLastError()) << Endl;
		return false;
	}
#endif

	ms_initialized++;
	return true;
}

void Network::finalize()
{
	T_ASSERT_M (ms_initialized > 0, L"Network not initialized");

	if (--ms_initialized > 0)
		return;

#if defined(_WIN32)
	WSACleanup();
#endif
}

	}
}
