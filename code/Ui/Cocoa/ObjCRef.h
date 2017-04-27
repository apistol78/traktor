/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/

#import <Cocoa/Cocoa.h>

#include "Core/Ref.h"
#include "Core/Object.h"

@interface ObjCRef : NSObject
{
	traktor::Object* m_ref;
}

- (id) init;

- (id) initWithRef: (traktor::Object*)ptr;

- (void) dealloc;

- (void) set: (traktor::Object*)ptr;

- (traktor::Object*) get;

@end
