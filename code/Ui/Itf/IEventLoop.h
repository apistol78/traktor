/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_IEventLoop_H
#define traktor_ui_IEventLoop_H

#include "Core/Config.h"
#include "Ui/Size.h"

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

	virtual int32_t execute(EventSubject* owner) = 0;
	
	virtual void exit(int32_t exitCode) = 0;

	virtual int32_t getExitCode() const = 0;

	virtual int32_t getAsyncKeyState() const = 0;

	virtual bool isKeyDown(VirtualKey vk) const = 0;

	virtual Size getDesktopSize() const = 0;
};

	}
}

#endif	// traktor_ui_IEventLoop_H
