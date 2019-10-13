#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Test/Case.h"
#include "Core/Test/TestClassFactory.h"

namespace traktor
{
    namespace test
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.TestClassFactory", 0, TestClassFactory, IRuntimeClassFactory)

void TestClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classCase = new AutoRuntimeClass< Case >();
	classCase->addMethod("execute", &Case::execute);
	registrar->registerClass(classCase);
}

    }
}
