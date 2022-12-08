/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/IEnvironment.h"
#include "Runtime/UpdateInfo.h"
#include "Runtime/Engine/Layer.h"
#include "Runtime/Engine/RuntimePlugin.h"
#include "Runtime/Engine/Stage.h"
#include "Runtime/Engine/StageData.h"
#include "Runtime/Engine/StageState.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.RuntimePlugin", 0, RuntimePlugin, IRuntimePlugin)

bool RuntimePlugin::create(IEnvironment* environment)
{
	return true;
}

void RuntimePlugin::destroy(IEnvironment* environment)
{
}

Ref< IState > RuntimePlugin::createInitialState(IEnvironment* environment)
{
	const Guid startupGuid(environment->getSettings()->getProperty< std::wstring >(L"Runtime.Startup"));

	Ref< StageData > stageData = environment->getDatabase()->getObjectReadOnly< StageData >(startupGuid);
	if (!stageData)
	{
		log::error << L"Unable to read startup stage, \"" << startupGuid.format() << L"\"; startup failed." << Endl;
		return nullptr;
	}

	Ref< Stage > stage = stageData->createInstance(environment, nullptr);
	if (!stage)
	{
		log::error << L"Unable to create startup stage, \"" << startupGuid.format() << L"\"; startup failed." << Endl;
		return nullptr;
	}

	// Prepare all initial layers; this will cause pending resources and systems
	// to be created before first frame.
	const UpdateInfo info = {};
	for (const auto layer : stage->getLayers())
		layer->preUpdate(info);

	return new StageState(environment, stage);
}

}
