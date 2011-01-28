#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

#include <Core/Ref.h>

namespace traktor
{

class Thread;

	namespace amalgam
	{

class Application;

	}
}

@interface EAGLView : UIView
{    
@private
	traktor::Ref< traktor::amalgam::Application > m_application;
	traktor::Thread* m_thread;
}

- (void) startAnimation;
- (void) stopAnimation;
- (void) drawView:(id)sender;

@end
