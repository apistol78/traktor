#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Core/Test/Case.h"
#include "Core/Test/TestClassFactory.h"

namespace traktor::test
{
	namespace
	{

class ReportDelegate : public Case::IReport
{
public:
	explicit ReportDelegate(IRuntimeDelegate* delegateReport)
	:   m_delegateReport(delegateReport)
	{
	}

	virtual void report(const std::wstring& message) const
	{
		Any argv[] =
		{
			CastAny< const std::wstring& >::set(message)
		};
		if (m_delegateReport)
			m_delegateReport->call(sizeof_array(argv), argv);
	}

private:
	IRuntimeDelegate* m_delegateReport;
};

bool Case_execute(Case* self, IRuntimeDelegate* delegateInfoReport, IRuntimeDelegate* delegateErrorReport)
{
	ReportDelegate infoReport(delegateInfoReport);
	ReportDelegate errorReport(delegateErrorReport);
	return self->execute(infoReport, errorReport);
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.TestClassFactory", 0, TestClassFactory, IRuntimeClassFactory)

void TestClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classCase = new AutoRuntimeClass< Case >();
	classCase->addMethod("execute", &Case_execute);
	registrar->registerClass(classCase);
}

}
