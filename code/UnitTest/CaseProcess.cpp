#include "UnitTest/CaseProcess.h"
#include "Core/Heap/Ref.h"
#include "Core/Io/Path.h"
#include "Core/System/OS.h"
#include "Core/System/Process.h"

namespace traktor
{

void CaseProcess::run()
{
	{
		Ref< Process > process = OS::getInstance().execute(L"$(TRAKTOR_HOME)/bin/unittest-process-1.bat", L"", L"", false);
		CASE_ASSERT(process != 0);

		if (process)
		{
			bool result = process->wait();
			CASE_ASSERT_EQUAL(result, true);

			int32_t exitCode = process->exitCode();
			CASE_ASSERT_EQUAL(exitCode, 0);
		}
	}

	{
		Ref< Process > process = OS::getInstance().execute(L"$(TRAKTOR_HOME)/bin/unittest-process-2.bat", L"Hello", L"", false);
		CASE_ASSERT(process != 0);

		if (process)
		{
			bool result = process->wait();
			CASE_ASSERT_EQUAL(result, true);

			int32_t exitCode = process->exitCode();
			CASE_ASSERT_EQUAL(exitCode, 0);
		}
	}
}

}
