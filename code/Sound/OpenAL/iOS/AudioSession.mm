/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>

#include "Core/Log/Log.h"
#include "Sound/OpenAL/iOS/AudioSession.h"

namespace traktor
{
	namespace sound
	{

void activateAudioSession()
{
	if ([UIApplication sharedApplication].applicationState == UIApplicationStateActive)
	{
		log::error << L"OpenAL error detected; trying to activate AV session..." << Endl;

		// Stupid iOS; after a phone call OpenAL keeps failing, so we try
		// to force our audio session to active.
		[[AVAudioSession sharedInstance] setActive: YES error: nil];
	}
	else
		log::error << L"OpenAL error detected; application inactive." << Endl;
}

	}
}
