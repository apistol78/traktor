/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/TheaterClassFactory.h"

#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Theater/TheaterEntityComponent.h"
#include "Theater/TheaterWorldComponent.h"

namespace traktor::theater
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.TheaterClassFactory", 0, TheaterClassFactory, IRuntimeClassFactory)

void TheaterClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classTheaterWorldComponent = new AutoRuntimeClass< TheaterWorldComponent >();
	classTheaterWorldComponent->addProperty("playing", &TheaterWorldComponent::isPlaying);
	classTheaterWorldComponent->addMethod("play", &TheaterWorldComponent::play);
	classTheaterWorldComponent->addMethod("stop", &TheaterWorldComponent::stop);
	registrar->registerClass(classTheaterWorldComponent);

	auto classTheaterEntityComponent = new AutoRuntimeClass< TheaterEntityComponent >();
	classTheaterEntityComponent->addProperty("playing", &TheaterEntityComponent::isPlaying);
	classTheaterEntityComponent->addMethod("play", &TheaterEntityComponent::play);
	classTheaterEntityComponent->addMethod("stop", &TheaterEntityComponent::stop);
	registrar->registerClass(classTheaterEntityComponent);
}

}
