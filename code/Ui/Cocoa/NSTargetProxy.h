
#import <Cocoa/Cocoa.h>

namespace traktor
{
	namespace ui
	{
	
struct ITargetProxyCallback
{
	virtual void targetProxy_Action(void* controlId) = 0;
};

	}
}

@interface NSTargetProxy : NSObject
{
	traktor::ui::ITargetProxyCallback* m_callback;
}

- (void) setCallback: (traktor::ui::ITargetProxyCallback*)callback;

- (void) dispatchActionCallback: (id)controlId;

@end
