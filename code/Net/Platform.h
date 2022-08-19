#ifndef traktor_net_Platform_H
#define traktor_net_Platform_H

#include "Core/Config.h"

#if defined(__GNUC__) || defined(__APPLE__)
#	include <sys/time.h>
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <sys/select.h>
#	include <netinet/in.h>
#	include <netinet/tcp.h>
#	include <arpa/inet.h>
#	include <netdb.h>
#	include <unistd.h>

#	define SOCKET int
#	define INVALID_SOCKET -1
#	define CLOSE_SOCKET(s) ::close(s)

#	if !defined(SOMAXCONN)
#		define SOMAXCONN 4
#	endif
#elif defined(_WIN32)
#	define _WIN32_LEAN_AND_MEAN
#	define NOMINMAX
#	include <winsock2.h>
#	include <ws2tcpip.h>
#	include <windows.h>

typedef int socklen_t;
#	define CLOSE_SOCKET(s) closesocket(s)

#endif

#endif	// traktor_net_Platform_H

