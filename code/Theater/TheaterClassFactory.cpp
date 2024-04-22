/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Theater/TheaterClassFactory.h"
#include "Theater/TheaterComponent.h"

namespace traktor::theater
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.TheaterClassFactory", 0, TheaterClassFactory, IRuntimeClassFactory)

void TheaterClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classTheaterComponent = new AutoRuntimeClass< TheaterComponent >();
	classTheaterComponent->addProperty("playing", &TheaterComponent::isPlaying);
	classTheaterComponent->addMethod("play", &TheaterComponent::play);
	classTheaterComponent->addMethod("stop", &TheaterComponent::stop);
	registrar->registerClass(classTheaterComponent);
}

}
