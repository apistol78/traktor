#include <cmath>
#include "Core/Thread/JobManager.h"
#include "Core/Test/CaseJob.h"

namespace traktor::test
{
	namespace
	{

std::atomic< int32_t > g_job;
std::atomic< int32_t > g_active;
std::atomic< int32_t > g_counts[1000];

void jobTask(int32_t index)
{
	g_active++;
	g_counts[index]++;
	g_job++;
	g_active--;
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseJob", 0, CaseJob, Case)

void CaseJob::run()
{
	// Fork all jobs.
	{
		g_job = 0;
		g_active = 0;

		Job::task_t jobs[1000];
		for (int32_t i = 0; i < 1000; ++i)
		{
			g_counts[i] = 0;
			jobs[i] = [=](){ jobTask(i); };
		}

		JobManager::getInstance().fork(jobs, sizeof_array(jobs));

		CASE_ASSERT_EQUAL((int32_t)g_active, 0);
		CASE_ASSERT_EQUAL((int32_t)g_job, 1000);

		bool correct = true;
		for (int32_t i = 0; i < 1000; ++i)
			correct &= (g_counts[i] == 1);
		CASE_ASSERT(correct);
	}

	// Add job one by one and then wait.
	{
		g_job = 0;
		g_active = 0;

		for (int32_t i = 0; i < 1000; ++i)
		{
			g_counts[i] = 0;
			JobManager::getInstance().add([=](){ jobTask(i); });
		}

		bool result = JobManager::getInstance().wait();
		CASE_ASSERT(result);

		CASE_ASSERT_EQUAL((int32_t)g_active, 0);
		CASE_ASSERT_EQUAL((int32_t)g_job, 1000);

		bool correct = true;
		for (int32_t i = 0; i < 1000; ++i)
			correct &= (g_counts[i] == 1);
		CASE_ASSERT(correct);
	}
}

}
