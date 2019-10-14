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
{
}

bool Case::execute(const IReport& report)
{
	m_report = &report;
	run();
	m_report = nullptr;
	return !m_failed;
}

void Case::succeeded(const std::wstring& message)
{
	if (m_report)
		m_report->report(true, message);
	else
		log::info << message << Endl;
}

void Case::failed(const std::wstring& message)
{
	if (m_report)
		m_report->report(false, message);
	else
		log::error << message << Endl;

	m_failed = true;
}

	}
}
