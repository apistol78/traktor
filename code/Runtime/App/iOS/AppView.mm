/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <dispatch/dispatch.h>

#import "Runtime/App/iOS/AppView.h"

#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Runtime/Impl/Application.h"
#include "Xml/XmlDeserializer.h"

using namespace traktor;

namespace
{

Ref< PropertyGroup > loadSettings(const Path& settingsPath)
{
	Ref< PropertyGroup > settings;
	Ref< IStream > file = FileSystem::getInstance().open(settingsPath, File::FmRead);
	if (file)
	{
		settings = xml::XmlDeserializer(file, settingsPath.getPathName()).readObject< PropertyGroup >();
		file->close();
	}
	return settings;
}

bool g_animation = false;
bool g_suspend = false;

}

@interface AppView ()
{
@private
}

@end

@implementation AppView

+ (Class) layerClass
{
    return [CAMetalLayer class];
}

- (BOOL) createApplication
{
	[self performSelectorOnMainThread:@selector(mainLoop) withObject:nil waitUntilDone:NO];
	return YES;
}

- (void) mainLoop
{
	SystemApplication sysapp;
	SystemWindow syswin(self);

	// Load settings.
	Ref< PropertyGroup > defaultSettings = loadSettings(L"Application.config");
	if (!defaultSettings)
	{
		traktor::log::error << L"Unable to read application settings \"Application.config\"" << Endl;
		return;
	}

	// "Activate" retina display if application want's to use it.
	if (defaultSettings->getProperty< bool >(L"Runtime.SupportRetina", false))
	{
		// Adjust scale as we want full resolution of a retina display.
		float scale = 1.0f;

		// Primarily use "nativeScale" as we don't want iPhone 6+ downscaling;
		// in case non iOS 8 we use ordinary "scale" property.
		if ([[UIScreen mainScreen] respondsToSelector:@selector(nativeScale)])
			scale = [UIScreen mainScreen].nativeScale;
		else
			scale = [UIScreen mainScreen].scale;

		// Limit scaling since too high isn't benefitial.
		if (scale > 2.0f)
			scale = 2.0f;

		traktor::log::info << L"Using content scale factor of " << scale << L"." << Endl;
		self.contentScaleFactor = scale;

		CAMetalLayer* metalLayer = (CAMetalLayer*)self.layer;
		if (metalLayer)
			metalLayer.contentsScale = scale;
	}

	// As we doesn't need to store user defined settings on iOS we
	// create a plain copy of the default settings.
	Ref< PropertyGroup > settings = DeepClone(defaultSettings).create< PropertyGroup >();
	T_FATAL_ASSERT (settings);

	// Create application.
	Ref< runtime::Application > application = new runtime::Application();
	if (!application->create(
		defaultSettings,
		settings,
		sysapp,
		&syswin
	))
		return;

	bool suspended = false;
	while (true)
	{
		{
			SInt32 result;
			do
			{
				result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, TRUE);
			}
			while (result == kCFRunLoopRunHandledSource);
		}

		if (g_suspend != suspended)
		{
			if (g_suspend)
				application->suspend();
			else
				application->resume();

			suspended = g_suspend;
		}

		if (g_animation && !suspended)
		{
			if (!application->update())
				break;
		}
	}

	safeDestroy(application);
}

- (void) drawView:(id)sender
{
}

- (void) layoutSubviews
{
}

- (void) startAnimation
{
	g_animation = true;
}

- (void) stopAnimation
{
	g_animation = false;
}

- (void) suspend
{
	g_suspend = true;
}

- (void) resume
{
	g_suspend = false;
}

- (void) dealloc
{
	[self stopAnimation];
    [super dealloc];
}

@end
