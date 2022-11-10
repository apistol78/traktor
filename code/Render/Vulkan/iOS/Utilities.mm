/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#import <UIKit/UIKit.h>

#include "Render/Vulkan/iOS/Utilities.h"

namespace traktor
{
    namespace render
    {

int32_t getScreenWidth()
{
	return [UIScreen mainScreen].bounds.size.width;
}

int32_t getScreenHeight()
{
	return [UIScreen mainScreen].bounds.size.height;
}

int32_t getViewWidth(void* view_)
{
	UIView* view = (UIView*)view_;
	float scale = [view contentScaleFactor];
	CGRect bounds = [view bounds];
	return (int32_t)(bounds.size.width * scale);
}

int32_t getViewHeight(void* view_)
{
	UIView* view = (UIView*)view_;
	float scale = [view contentScaleFactor];
	CGRect bounds = [view bounds];
	return (int32_t)(bounds.size.height * scale);
}

    }
}