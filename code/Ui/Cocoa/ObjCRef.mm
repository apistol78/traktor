#import "Ui/Cocoa/ObjCRef.h"

@implementation ObjCRef

- (id) initWithRef: (traktor::Object*)ptr
{
	m_ref = new traktor::Ref< traktor::Object >(ptr);
	return self;
}

- (void) set: (traktor::Object*)ptr
{
	delete m_ref;
	m_ref = new traktor::Ref< traktor::Object >(ptr);
}

- (traktor::Object*) get
{
	return m_ref ? *m_ref : 0;
}

@end
