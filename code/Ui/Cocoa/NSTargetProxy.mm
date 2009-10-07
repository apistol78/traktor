#import "Ui/Cocoa/NSTargetProxy.h"

@implementation NSTargetProxy

- (void) setCallback: (traktor::ui::ITargetProxyCallback*)callback
{
	m_callback = callback;
}

- (void) dispatchActionCallback
{
	m_callback->targetProxy_Action();
}

@end
