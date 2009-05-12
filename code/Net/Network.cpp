#include "Net/Network.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace net
	{

bool Network::ms_initialized = false;

bool Network::initialize()
{
	if (ms_initialized)
		return true;

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
		return false;
	}

#endif

	ms_initialized = true;
	return true;
}

void Network::finalize()
{
	if (!ms_initialized)
		return;

#if defined(_WIN32)

	WSACleanup();

#elif defined(_PS3)

	sys_net_finalize_network();

#endif

#if defined(_XBOX)

	XNetCleanup();

#endif

	ms_initialized = false;
}

	}
}
