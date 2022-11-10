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
#include "Theater/TheaterClassFactory.h"
#include "Theater/TheaterController.h"

namespace traktor
{
	namespace theater
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.TheaterClassFactory", 0, TheaterClassFactory, IRuntimeClassFactory)

void TheaterClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classTheaterController = new AutoRuntimeClass< TheaterController >();
	classTheaterController->addMethod("play", &TheaterController::play);
	registrar->registerClass(classTheaterController);
}

	}
}
