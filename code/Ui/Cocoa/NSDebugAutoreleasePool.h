/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_NSDebugAutoreleasePool_H
#define traktor_ui_NSDebugAutoreleasePool_H

#import <Cocoa/Cocoa.h>

@interface NSDebugAutoreleasePool : NSAutoreleasePool

- (void) addObject: (id)object;

@end

#endif	// traktor_ui_NSDebugAutoreleasePool_H
