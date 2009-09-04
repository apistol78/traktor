#ifndef traktor_ui_IEventLoop_H
#define traktor_ui_IEventLoop_H

#include "Core/Config.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

/*! \brief EventLoop interface.
 * \ingroup UI
 */
class IEventLoop
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
