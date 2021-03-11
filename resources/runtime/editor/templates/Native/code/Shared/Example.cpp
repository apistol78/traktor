#include <Core/Log/Log.h>
#include "Shared/Example.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"shared.Example", Example, traktor::Object)

void Example::sayHelloWorld()
{
	log::info << L"Hello World!" << Endl;
}
