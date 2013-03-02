#include "Amalgam/IEnvironment.h"
#include "Core/Functor/Functor.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Database/Database.h"
#include "Parade/StageData.h"
#include "Parade/StageLoader.h"

namespace traktor
{
	namespace parade
	{
		namespace
		{

void jobLoader(amalgam::IEnvironment* environment, const Guid& stageGuid, Ref< const Object > params, Ref< StageLoader > stageLoader, Ref< Stage >& outStage)
{
	Ref< StageData > stageData = environment->getDatabase()->getObjectReadOnly< StageData >(stageGuid);
	if (!stageData)
		return;

	outStage = stageData->createInstance(environment, params);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.StageLoader", StageLoader, Object)

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

Ref< StageLoader > StageLoader::createAsync(amalgam::IEnvironment* environment, const Guid& stageGuid, const Object* params)
{
	Ref< StageLoader > stageLoader = new StageLoader();

	stageLoader->m_job = JobManager::getInstance().add(makeStaticFunctor< amalgam::IEnvironment*, const Guid&, Ref< const Object >, Ref< StageLoader >, Ref< Stage >& >(&jobLoader, environment, stageGuid, params, stageLoader, stageLoader->m_stage));
	if (!stageLoader->m_job)
		return 0;

	return stageLoader;
}

Ref< StageLoader > StageLoader::create(amalgam::IEnvironment* environment, const Guid& stageGuid, const Object* params)
{
	Ref< StageLoader > stageLoader = new StageLoader();
	jobLoader(environment, stageGuid, params, stageLoader, stageLoader->m_stage);
	return stageLoader;
}


	}
}
