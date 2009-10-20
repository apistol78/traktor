#ifndef traktor_ui_NSDebugAutoreleasePool_H
#define traktor_ui_NSDebugAutoreleasePool_H

#import <Cocoa/Cocoa.h>

@interface NSDebugAutoreleasePool : NSAutoreleasePool

- (void) addObject: (id)object;

@end

#endif	// traktor_ui_NSDebugAutoreleasePool_H
