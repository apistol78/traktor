#ifndef traktor_render_Error_H
#define traktor_render_Error_H

#include "Core/Debug/Debugger.h"
#include "Core/Log/Log.h"

#define T_CAPTURE_ASSERT(condition, message) \
	if (!(condition)) { \
		log::error << L"Render error: " << message << Endl; \
		Debugger::getInstance().breakDebugger(); \
	}

#endif	// traktor_render_Error_H

