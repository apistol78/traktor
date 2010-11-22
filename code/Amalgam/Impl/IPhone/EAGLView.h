#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

#include <Core/Ref.h>

namespace amalgam
{
	class Application;
}

@interface EAGLView : UIView
{    
@private
	traktor::Ref< amalgam::Application > m_application;
	
	BOOL animating;
	BOOL displayLinkSupported;
	NSInteger animationFrameInterval;
	id displayLink;
    NSTimer *animationTimer;
}

@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic) NSInteger animationFrameInterval;

- (void) startAnimation;
- (void) stopAnimation;
- (void) drawView:(id)sender;

@end
