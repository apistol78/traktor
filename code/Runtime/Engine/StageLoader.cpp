/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Core/Timer/Timer.h"
#include "Database/Database.h"
#include "Runtime/IEnvironment.h"
#include "Runtime/Engine/StageData.h"
#include "Runtime/Engine/StageLoader.h"

namespace traktor::runtime
{
	namespace
	{

void jobLoader(IEnvironment* environment, Guid stageGuid, Ref< const Object > params, Ref< StageLoader > stageLoader, Ref< Stage >& outStage)
{
	Ref< StageData > stageData = environment->getDatabase()->getObjectReadOnly< StageData >(stageGuid);
	if (!stageData)
		return;

	Timer timer;
	outStage = stageData->createInstance(environment, params);
	log::info << L"Stage " << stageGuid.format() << L" loaded in " << formatDuration(timer.getElapsedTime()) << L"." << Endl;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.StageLoader", StageLoader, Object)

bool StageLoader::wait()
{
	return m_job ? m_job->wait() : true;
}

bool StageLoader::ready()
{
	return m_job ? m_job->stopped() : true;
}

bool StageLoader::succeeded()
{
	if (!wait())
		return false;

	return m_stage != nullptr;
}

bool StageLoader::failed()
{
	if (!wait())
		return false;

	return m_stage == nullptr;
}

Ref< Stage > StageLoader::get()
{
	if (!wait())
		return nullptr;

	return m_stage;
}

Ref< StageLoader > StageLoader::createAsync(IEnvironment* environment, const Guid& stageGuid, const Object* params)
{
	Ref< StageLoader > stageLoader = new StageLoader();
	Ref< const Object > paramsCapture = params;
	stageLoader->m_job = JobManager::getInstance().add([=](){
		jobLoader(environment, stageGuid, paramsCapture, stageLoader, stageLoader->m_stage);
	});
	if (stageLoader->m_job)
		return stageLoader;
	else
		return nullptr;
}

Ref< StageLoader > StageLoader::create(IEnvironment* environment, const Guid& stageGuid, const Object* params)
{
	Ref< StageLoader > stageLoader = new StageLoader();
	jobLoader(environment, stageGuid, params, stageLoader, stageLoader->m_stage);
	return stageLoader;
}


}
