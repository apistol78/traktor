/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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
