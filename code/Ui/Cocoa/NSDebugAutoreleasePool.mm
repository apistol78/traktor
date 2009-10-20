#include "Ui/Cocoa/NSDebugAutoreleasePool.h"
#include "Core/Log/Log.h"

@implementation NSDebugAutoreleasePool

- (void) addObject: (id)object
{
	[super addObject: object];
}

@end
