#include "Amalgam/Game/IEnvironment.h"
#include "Amalgam/Game/Engine/StageData.h"
#include "Amalgam/Game/Engine/StageLoader.h"
#include "Core/Functor/Functor.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Database/Database.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

void jobLoader(IEnvironment* environment, const Guid& stageGuid, Ref< const Object > params, Ref< StageLoader > stageLoader, Ref< Stage >& outStage)
{
	Ref< StageData > stageData = environment->getDatabase()->getObjectReadOnly< StageData >(stageGuid);
	if (!stageData)
		return;

	outStage = stageData->createInstance(environment, params);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.StageLoader", StageLoader, Object)

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

	return m_stage != 0;
}

bool StageLoader::failed()
{
	if (!wait())
		return false;

	return m_stage == 0;
}

Ref< Stage > StageLoader::get()
{
	if (!wait())
		return 0;

	return m_stage;
}

Ref< StageLoader > StageLoader::createAsync(IEnvironment* environment, const Guid& stageGuid, const Object* params)
{
#if !defined(__EMSCRIPTEN__)
	Ref< StageLoader > stageLoader = new StageLoader();
	stageLoader->m_job = JobManager::getInstance().add(makeStaticFunctor< IEnvironment*, const Guid&, Ref< const Object >, Ref< StageLoader >, Ref< Stage >& >(&jobLoader, environment, stageGuid, params, stageLoader, stageLoader->m_stage));
	if (stageLoader->m_job)
		return stageLoader;
	else
		return 0;
#else
	return create(environment, stageGuid, params);
#endif
}

Ref< StageLoader > StageLoader::create(IEnvironment* environment, const Guid& stageGuid, const Object* params)
{
	Ref< StageLoader > stageLoader = new StageLoader();
	jobLoader(environment, stageGuid, params, stageLoader, stageLoader->m_stage);
	return stageLoader;
}


	}
}
