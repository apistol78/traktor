#pragma once

#import <Cocoa/Cocoa.h>

@interface NSDebugAutoreleasePool : NSAutoreleasePool

- (void) addObject: (id)object;

@end

