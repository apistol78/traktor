/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Input/Android/InputAndroidClassFactory.h"
#include "Input/Android/KeyboardDeviceAndroid.h"
#include "Input/Android/MouseDeviceAndroid.h"
#include "Input/Android/SensorDeviceAndroid.h"
#include "Input/Android/TouchDeviceAndroid.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputAndroidClassFactory", 0, InputAndroidClassFactory, IRuntimeClassFactory)

void InputAndroidClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classKeyboardDeviceAndroid = new AutoRuntimeClass< KeyboardDeviceAndroid >();
	classKeyboardDeviceAndroid->addStaticMethod("showSoftKeyboard", &KeyboardDeviceAndroid::showSoftKeyboard);
	classKeyboardDeviceAndroid->addStaticMethod("hideSoftKeyboard", &KeyboardDeviceAndroid::hideSoftKeyboard);
	registrar->registerClass(classKeyboardDeviceAndroid);

	auto classMouseDeviceAndroid = new AutoRuntimeClass< MouseDeviceAndroid >();
	registrar->registerClass(classMouseDeviceAndroid);

	auto classSensorDeviceAndroid = new AutoRuntimeClass< SensorDeviceAndroid >();
	registrar->registerClass(classSensorDeviceAndroid);

	auto classTouchDeviceAndroid = new AutoRuntimeClass< TouchDeviceAndroid >();
	registrar->registerClass(classTouchDeviceAndroid);
}

	}
}
