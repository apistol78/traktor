#pragma once

#if defined(_WIN32) || defined(__LINUX__) || defined(__RPI__) || (defined(__APPLE__) && !defined(__IOS__))
#	define T_NEED_RUNTIME_SIGNATURE
#endif
