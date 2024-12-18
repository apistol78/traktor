/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#import <UIKit/UIKit.h>
#import "Runtime/App/iOS/LaunchAppDelegate.h"

int main(int argc, char* argv[])
{
	@autoreleasepool {
	    return UIApplicationMain(argc, argv, nil, NSStringFromClass([LaunchAppDelegate class]));
	}
}

