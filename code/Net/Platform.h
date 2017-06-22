/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_net_Platform_H
#define traktor_net_Platform_H

#include "Core/Config.h"

#if defined(__GNUC__) || defined(__PS3__) || defined(__APPLE__)
#	include <sys/time.h>
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <sys/select.h>
#	include <netinet/in.h>
#	include <netinet/tcp.h>
#	include <arpa/inet.h>
#	if !defined(__PS4__)
#		include <netdb.h>
#	endif
#	include <unistd.h>

#	if defined(__PS3__)
#		include <netex/net.h>
#		include <netex/ifctl.h>
#		include <netex/libnetctl.h>
#		include <cell/sysmodule.h>
#		include <sys/timer.h>
#		include <sysutil/sysutil_common.h>
#	endif

#	define SOCKET int
#	define INVALID_SOCKET -1

#	if defined(__PS3__)
#		define CLOSE_SOCKET(s) socketclose(s)
#	else
#		define CLOSE_SOCKET(s) ::close(s)
#	endif

#	if !defined(SOMAXCONN)
#		define SOMAXCONN 4
#	endif
#elif defined(_WIN32)
#	if defined(_XBOX)
#		include <xtl.h>
#	else
#		define _WIN32_LEAN_AND_MEAN
#		define NOMINMAX
#		include <winsock2.h>
#		include <ws2tcpip.h>
#		include <windows.h>
#	endif

typedef int socklen_t;
#	define CLOSE_SOCKET(s) closesocket(s)

#endif

#endif	// traktor_net_Platform_H

