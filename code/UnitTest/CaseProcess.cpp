#include "UnitTest/CaseProcess.h"
#include "Core/Io/Path.h"
#include "Core/System/OS.h"
#include "Core/System/IProcess.h"

namespace traktor
{

void CaseProcess::run()
{
	{
		Ref< IProcess > process = OS::getInstance().execute(L"$(TRAKTOR_HOME)/bin/unittest-process-1.bat", L"", 0, false, false, false);
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
		Ref< IProcess > process = OS::getInstance().execute(L"$(TRAKTOR_HOME)/bin/unittest-process-2.bat Hello", L"", 0, false, false, false);
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
