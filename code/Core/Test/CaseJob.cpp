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
int32_t g_active;
int32_t g_counts[1000];

void jobTask(int32_t index)
{
	Atomic::increment(g_active);
	Atomic::increment(g_counts[index]);
	Atomic::increment(g_job);
	Atomic::decrement(g_active);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseJob", 0, CaseJob, Case)

void CaseJob::run()
{
	// Fork all jobs.
	{
		g_job = 0;
		g_active = 0;

		RefArray< Functor > jobs;
		for (int32_t i = 0; i < 1000; ++i)
		{
			g_counts[i] = 0;
			jobs.push_back(makeStaticFunctor(jobTask, i));
		}

		JobManager::getInstance().fork(jobs);

		CASE_ASSERT_EQUAL(g_active, 0);
		CASE_ASSERT_EQUAL(g_job, 1000);

		bool correct = true;
		for (int32_t i = 0; i < 1000; ++i)
			correct &= (g_counts[i] == 1);
		CASE_ASSERT(correct);
	}

	// Add job on by on and wait.
	{
		g_job = 0;
		g_active = 0;

		for (int32_t i = 0; i < 1000; ++i)
		{
			g_counts[i] = 0;
			JobManager::getInstance().add(makeStaticFunctor(jobTask, i));
		}

		bool result = JobManager::getInstance().wait();

		CASE_ASSERT_EQUAL(g_active, 0);
		CASE_ASSERT_EQUAL(g_job, 1000);
		CASE_ASSERT(result);

		bool correct = true;
		for (int32_t i = 0; i < 1000; ++i)
			correct &= (g_counts[i] == 1);
		CASE_ASSERT(correct);
	}
}

	}
}
