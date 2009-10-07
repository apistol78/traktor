#import <Cocoa/Cocoa.h>

#include "Ui/Cocoa/EventLoopCocoa.h"
#include "Ui/Enums.h"

namespace traktor
{
	namespace ui
	{

EventLoopCocoa::EventLoopCocoa()
{
	m_pool = [[NSAutoreleasePool alloc] init];
	[NSApplication sharedApplication];
}

EventLoopCocoa::~EventLoopCocoa()
{
	[m_pool release];
}

bool EventLoopCocoa::process(EventSubject* owner)
{
	return false;
}

int EventLoopCocoa::execute(EventSubject* owner)
{
	[NSApp run];
	return 0;
}

void EventLoopCocoa::exit(int exitCode)
{
}

int EventLoopCocoa::getExitCode() const
{
	return 0;
}

int EventLoopCocoa::getAsyncKeyState() const
{
	return KsNone;
}

	}
}
