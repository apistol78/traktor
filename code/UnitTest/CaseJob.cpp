/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cmath>
#include "Core/Functor/Functor.h"
#include "Core/Log/Log.h"
#include "Core/Thread/Atomic.h"
#include "Core/Thread/JobManager.h"
#include "Core/Timer/Timer.h"
#include "UnitTest/CaseJob.h"

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

}

}
