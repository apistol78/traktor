#ifndef traktor_amalgam_LaunchAppDelegate_H
#define traktor_amalgam_LaunchAppDelegate_H

#import <UIKit/UIKit.h>

@class EAGLView;

@interface LaunchAppDelegate : NSObject <UIApplicationDelegate>
{
	UIWindow *window;
	EAGLView *glView;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet EAGLView *glView;

@end

#endif	// traktor_amalgam_LaunchAppDelegate_H
