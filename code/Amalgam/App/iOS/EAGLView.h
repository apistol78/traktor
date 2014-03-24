#ifndef traktor_amalgam_EAGLView_H
#define traktor_amalgam_EAGLView_H

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

- (BOOL) createApplication;

- (void) drawView:(id)sender;

- (void) layoutSubviews;

- (void) startAnimation;

- (void) stopAnimation;

- (void) dealloc;

@end

#endif	// traktor_amalgam_EAGLView_H
