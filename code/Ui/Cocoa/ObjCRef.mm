#import "Ui/Cocoa/ObjCRef.h"

using namespace traktor;

@implementation ObjCRef

- (id) init
{
	if ((self = [super init]) != nil)
		m_ref = 0;
	return self;
}

- (id) initWithRef: (traktor::Object*)ptr
{
	if ((self = [super init]) != nil)
	{
		T_SAFE_ADDREF(ptr);
		m_ref = ptr;
	}
	return self;
}

- (void) dealloc
{
	T_SAFE_RELEASE(m_ref); m_ref = 0;
	[super dealloc];
}

- (void) set: (traktor::Object*)ptr
{
	T_SAFE_RELEASE(m_ref);
	T_SAFE_ADDREF(ptr);
	m_ref = ptr;
}

- (traktor::Object*) get
{
	return m_ref;
}

@end
