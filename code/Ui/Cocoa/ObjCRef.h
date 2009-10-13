
#import <Cocoa/Cocoa.h>

#include "Core/Heap/Ref.h"
#include "Core/Object.h"

@interface ObjCRef : NSObject
{
	traktor::Ref< traktor::Object >* m_ref;
}

- (id) initWithRef: (traktor::Object*)ptr;

- (void) set: (traktor::Object*)ptr;

- (traktor::Object*) get;

@end
