#import "Ui/Cocoa/NSCustomOutlineView.h"

@implementation NSCustomOutlineView

- (void) rightMouseDown: (NSEvent*)theEvent
{
	id delegate = [self delegate];
	
	if ([delegate respondsToSelector:@selector(outlineViewRightMouseDown:)])
		[delegate outlineViewRightMouseDown: theEvent];
		
	[super rightMouseDown: theEvent];
}

@end
