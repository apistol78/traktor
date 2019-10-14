#include <cmath>
#include "Core/Functor/Functor.h"
#include "Core/Log/Log.h"
#include "Core/Thread/Atomic.h"
#include "Core/Thread/JobManager.h"
#include "Core/Timer/Timer.h"
#include "Core/Test/CaseJob.h"

namespace traktor
{
	namespace test
	{
		namespace
		{

int32_t g_job;

void jobTask()
{
	Atomic::increment(g_job);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseJob", 0, CaseJob, Case)

void CaseJob::run()
{
	// Fork all jobs.
	{
		g_job = 0;

		RefArray< Functor > jobs;
		for (int32_t i = 0; i < 1000; ++i)
			jobs.push_back(makeStaticFunctor(jobTask));

		JobManager::getInstance().fork(jobs);

		CASE_ASSERT_EQUAL(g_job, 1000);
	}

	// Add job on by on and wait.
	{
		g_job = 0;

		for (int32_t i = 0; i < 1000; ++i)
			JobManager::getInstance().add(makeStaticFunctor(jobTask));

		JobManager::getInstance().wait();

		CASE_ASSERT_EQUAL(g_job, 1000);
	}
}

	}
}
