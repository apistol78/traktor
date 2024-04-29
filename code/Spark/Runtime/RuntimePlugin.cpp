/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/IEnvironment.h"
#include "Spark/MovieResourceFactory.h"
#include "Spark/Runtime/RuntimePlugin.h"
#include "Resource/IResourceManager.h"

namespace traktor::spark
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.RuntimePlugin", 0, RuntimePlugin, runtime::IRuntimePlugin)

bool RuntimePlugin::create(runtime::IEnvironment* environment)
{
	auto resourceManager = environment->getResource()->getResourceManager();
	resourceManager->addFactory(new MovieResourceFactory());
	return true;
}

void RuntimePlugin::destroy(runtime::IEnvironment* environment)
{
}

Ref< runtime::IState > RuntimePlugin::createInitialState(runtime::IEnvironment* environment)
{
	return nullptr;
}

}
