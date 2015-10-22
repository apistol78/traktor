#ifndef traktor_amalgam_EAGLView_H
#define traktor_amalgam_EAGLView_H

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

#include <Core/Ref.h>

namespace traktor
{

class Thread;

}

@interface EAGLView : UIView

- (BOOL) createApplication;

- (void) drawView:(id)sender;

- (void) layoutSubviews;

- (void) startAnimation;

- (void) stopAnimation;

- (void) suspend;

- (void) resume;

- (void) dealloc;

@end

#endif	// traktor_amalgam_EAGLView_H
