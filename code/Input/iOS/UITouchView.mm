#import "Input/iOS/UITouchView.h"

@implementation UITouchView

- (void) setCallback: (traktor::input::ITouchViewCallback*) callback
{
	m_callback = callback;
}

- (void) touchesBegan: (NSSet*) touches withEvent: (UIEvent*) event
{
	if (m_callback)
		m_callback->touchesBegan(touches, event);
}

- (void) touchesMoved: (NSSet*) touches withEvent: (UIEvent*) event
{
	if (m_callback)
		m_callback->touchesMoved(touches, event);
}

- (void) touchesEnded: (NSSet*) touches withEvent: (UIEvent*) event
{
	if (m_callback)
		m_callback->touchesEnded(touches, event);
}

- (void) touchesCancelled: (NSSet*) touches withEvent: (UIEvent*) event
{
	if (m_callback)
		m_callback->touchesCancelled(touches, event);
}

@end
