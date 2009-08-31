#include "UnitTest/Case.h"
#include "Core/Log/Log.h"

namespace traktor
{

void Case::succeeded(const std::wstring& message)
{
	log::info << message << Endl;
}

void Case::failed(const std::wstring& message)
{
	log::error << message << Endl;
}

}
