#import <UIKit/UIKit.h>

#include "Render/Vulkan/iOS/Utilities.h"

namespace traktor
{
    namespace render
    {

int32_t getViewWidth(void* view_)
{
	UIView* view = (UIView*)view_;
	CGRect bounds = [view bounds];
	return bounds.size.width;
}

int32_t getViewHeight(void* view_)
{
	UIView* view = (UIView*)view_;
	CGRect bounds = [view bounds];
	return bounds.size.height;   
}

    }
}