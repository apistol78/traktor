#pragma once

#include "Core/Debug/Debugger.h"
#include "Core/Log/Log.h"

#define T_CAPTURE_ASSERT(condition, message) \
	if (!(condition)) { \
		log::error << L"Render error: " << message << Endl; \
		Debugger::getInstance().breakDebugger(); \
	}

#if 0
#	define T_CAPTURE_TRACE(method) \
		log::debug << method << Endl;
#else
#	define T_CAPTURE_TRACE(method)
#endif

