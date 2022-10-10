#include "Core/CycleRefDebugger.h"
#include "Core/Test/CaseCycleDebugger.h"

namespace traktor::test
{
	namespace
	{

class DummyWrapper
{
public:
	Ref< Object > m_ref;
};

class FooClass : public Object
{
public:
	std::map< std::wstring, DummyWrapper > m_dummy;
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseCycleDebugger", 0, CaseCycleDebugger, Case)

void CaseCycleDebugger::run()
{
	CycleRefDebugger cycleDebugger;
	Object::setReferenceDebugger(&cycleDebugger);

	{
		Ref< FooClass > foo1 = new FooClass();
		foo1->m_dummy[L"MyRef"].m_ref = foo1;
	}

	Object::setReferenceDebugger(0);
}

}
