/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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

#if defined(_XBOX)

	XNetStartupParams xnsp;
	memset(&xnsp, 0, sizeof(xnsp));
	xnsp.cfgSizeOfStruct = sizeof(XNetStartupParams);
	xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;

	if (XNetStartup(&xnsp) != 0)
		return false;
#endif

#if defined(_WIN32)

	static WSADATA s_wsa;
	if (WSAStartup(MAKEWORD(2, 2), &s_wsa) != 0)
	{
		log::error << L"WSAStartup failed; GetLastError = " << uint32_t(GetLastError()) << Endl;
		return false;
	}

#elif defined(_PS3)

	int ret = cellSysmoduleLoadModule(CELL_SYSMODULE_NET);
	if (ret < 0)
	{
		log::error << L"cellSysmoduleLoadModule failed, " << ret << Endl;
		return false;
	}

	sys_net_initialize_network();

	ret = cellNetCtlInit();
	if (ret < 0)
	{
		log::error << L"cellNetCtlInit failed, " << ret << Endl;
		return false;
	}

	int state;
	for(int i = 0; i < 10; ++i)
	{
		ret = cellNetCtlGetState(&state);
		if (ret < 0)
		{
			log::error << L"cellNetCtlGetState failed, " << ret << Endl;
			return false;
		}

		if (state == CELL_NET_CTL_STATE_IPObtained)
			break;

		sys_timer_usleep(500 * 1000);
	}

	if (state != CELL_NET_CTL_STATE_IPObtained)
	{
		log::error << L"No IP obtained, check cables" << Endl;
		return true;
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

#elif defined(_PS3)

	sys_net_finalize_network();

#endif

#if defined(_XBOX)

	XNetCleanup();

#endif
}

	}
}
