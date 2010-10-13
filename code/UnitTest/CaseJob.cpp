#include <cmath>
#include "UnitTest/CaseJob.h"
#include "Core/Thread/JobManager.h"
#include "Core/Thread/Atomic.h"
#include "Core/Timer/Timer.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace
	{

int32_t g_job;

void job_1()
{
	Atomic::increment(g_job);
}

void job_2()
{
	Atomic::increment(g_job);
}

void job_3()
{
	Atomic::increment(g_job);
}

volatile float g_dummy[800000];

void job_stress(int offset)
{
	for (int i = 0; i < 200000; ++i)
	{
		g_dummy[i + offset] = std::sqrt(float(i + 1));
	}
}

	}

void CaseJob::run()
{
	{
		g_job = 0;

		RefArray< Functor > jobs(3);
		jobs[0] = makeStaticFunctor(job_1);
		jobs[1] = makeStaticFunctor(job_2);
		jobs[2] = makeStaticFunctor(job_3);
		JobManager::getInstance().fork(jobs);

		CASE_ASSERT_EQUAL(g_job, 3);
	}

	// Job improvement case, might not be successful on single core.
	// Overhead of JobManager is 0.05 ms on a Dell 690 Quad.
	{
		Timer timerJob;
		timerJob.start();

		RefArray< Functor > jobs(4);
		jobs[0] = makeStaticFunctor(job_stress, 0);
		jobs[1] = makeStaticFunctor(job_stress, 200000);
		jobs[2] = makeStaticFunctor(job_stress, 400000);
		jobs[3] = makeStaticFunctor(job_stress, 600000);
		JobManager::getInstance().fork(jobs);

		timerJob.stop();
		double timeJob = timerJob.getElapsedTime();

		Timer timerSingle;
		timerSingle.start();

		job_stress(0);
		job_stress(200000);
		job_stress(400000);
		job_stress(600000);

		timerSingle.stop();
		double timeSingle = timerSingle.getElapsedTime();

		log::info << L"Time single " << timeSingle * 1000 << L" ms, job " << timeJob * 1000 << Endl;

		CASE_ASSERT(timeJob < timeSingle * 0.4);
	}
}

}
