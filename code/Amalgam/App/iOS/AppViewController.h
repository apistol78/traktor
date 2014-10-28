#ifndef traktor_amalgam_AppViewController_H
#define traktor_amalgam_AppViewController_H

#import <UIKit/UIKit.h>

@interface AppViewController : UIViewController

- (id) init;

- (void) loadView;

- (void) viewDidLoad;

- (BOOL) prefersStatusBarHidden;

- (void) startAnimation;

- (void) stopAnimation;

@end

#endif	// traktor_amalgam_AppViewController_H

