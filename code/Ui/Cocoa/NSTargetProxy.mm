#import "Ui/Cocoa/NSTargetProxy.h"

@implementation NSTargetProxy

- (id) init
{
	if ((self = [super init]) != nil)
		m_callback = 0;
	return self;
}

- (void) setCallback: (traktor::ui::ITargetProxyCallback*)callback
{
	m_callback = callback;
}

- (void) dispatchActionCallback: (id)controlId
{
	m_callback->targetProxy_Action(controlId);
}

- (void) dispatchDoubleActionCallback: (id)controlId
{
	m_callback->targetProxy_doubleAction(controlId);
}

@end
