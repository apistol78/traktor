/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/CycleRefDebugger.h"
#include "Core/RefArray.h"
#include "UnitTest/CaseCycleDebugger.h"

namespace traktor
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
