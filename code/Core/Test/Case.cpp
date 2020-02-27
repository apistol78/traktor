#include "Core/Log/Log.h"
#include "Core/Test/Case.h"

namespace traktor
{
	namespace test
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.test.Case", Case, Object)

Case::Case()
:	m_report(nullptr)
,	m_failed(false)
,	m_allocdelta(0)
{
}

bool Case::execute(const IReport& report)
{
	m_report = &report;
	m_allocdelta = 0;

	int64_t allocPre = (int64_t)Alloc::allocated();

	run();

	int64_t allocPost = (int64_t)Alloc::allocated();
	int64_t allocBalance = (allocPost - allocPre) - m_allocdelta;
	
	if (allocBalance != 0)
	{
		StringOutputStream ss;
		ss << T_FILE_LINE_W << L" \"" << type_name(this) << L"\" failed; " << allocBalance << L" memory allocation leak."; \
		failed(ss.str());
	}

	m_report = nullptr;
	return !m_failed;
}

void Case::succeeded(const std::wstring& message)
{
	int64_t allocPre = (int64_t)Alloc::allocated();

	if (m_report)
		m_report->report(true, message);
	else
		log::info << message << Endl;

	int64_t allocPost = (int64_t)Alloc::allocated();
	m_allocdelta += (allocPost - allocPre);
}

void Case::failed(const std::wstring& message)
{
	int64_t allocPre = (int64_t)Alloc::allocated();

	if (m_report)
		m_report->report(false, message);
	else
		log::error << message << Endl;

	int64_t allocPost = (int64_t)Alloc::allocated();
	m_allocdelta += (allocPost - allocPre);

	m_failed = true;
}

	}
}
