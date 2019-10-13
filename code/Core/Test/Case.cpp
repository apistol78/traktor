#include "Core/Log/Log.h"
#include "Core/Test/Case.h"

namespace traktor
{
	namespace test
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.test.Case", Case, Object)

Case::Case()
:	m_failed(false)
{
}

bool Case::execute()
{
	run();
	return !m_failed;
}

void Case::succeeded(const std::wstring& message)
{
	log::info << message << Endl;
}

void Case::failed(const std::wstring& message)
{
	log::error << message << Endl;
	m_failed = true;
}

	}
}
