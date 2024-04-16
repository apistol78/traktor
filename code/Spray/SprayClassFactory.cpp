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
#include "Spray/EffectComponent.h"
#include "Spray/ListenerComponent.h"
#include "Spray/SoundComponent.h"
#include "Spray/SprayClassFactory.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.SprayClassFactory", 0, SprayClassFactory, IRuntimeClassFactory)

void SprayClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classEffectComponent = new AutoRuntimeClass< EffectComponent >();
	classEffectComponent->addProperty("loopEnable", &EffectComponent::setLoopEnable, &EffectComponent::getLoopEnable);
	classEffectComponent->addProperty("finished", &EffectComponent::isFinished);
	classEffectComponent->addProperty("enable", &EffectComponent::setEnable, &EffectComponent::isEnable);
	classEffectComponent->addMethod("reset", &EffectComponent::reset);
	registrar->registerClass(classEffectComponent);

	auto classListenerComponent = new AutoRuntimeClass< ListenerComponent >();
	classListenerComponent->addProperty("enable", &ListenerComponent::setEnable, &ListenerComponent::isEnable);
	registrar->registerClass(classListenerComponent);

	auto classSoundComponent = new AutoRuntimeClass< SoundComponent >();
	classSoundComponent->addMethod("play", &SoundComponent::play);
	classSoundComponent->addMethod("stop", &SoundComponent::stop);
	classSoundComponent->addMethod("setVolume", &SoundComponent::setVolume);
	classSoundComponent->addMethod("setPitch", &SoundComponent::setPitch);
	classSoundComponent->addMethod("setParameter", &SoundComponent::setParameter);
	registrar->registerClass(classSoundComponent);
}

}
