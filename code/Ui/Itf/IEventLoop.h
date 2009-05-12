#ifndef traktor_ui_IEventLoop_H
#define traktor_ui_IEventLoop_H

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class EventSubject;

/*! \brief EventLoop interface.
 * \ingroup UI
 */
class T_DLLCLASS IEventLoop
{
public:
	virtual ~IEventLoop() {}

	virtual bool process(EventSubject* owner) = 0;

	virtual int execute(EventSubject* owner) = 0;
	
	virtual void exit(int exitCode) = 0;

	virtual int getExitCode() const = 0;

	virtual int getAsyncKeyState() const = 0;
};

	}
}

#endif	// traktor_ui_IEventLoop_H
