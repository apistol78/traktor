/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Test/MathCompare.h"
#include "Jungle/State/BodyStateTemplate.h"
#include "Jungle/State/BodyStateValue.h"
#include "Jungle/State/FloatTemplate.h"
#include "Jungle/State/FloatValue.h"
#include "Jungle/State/State.h"
#include "Jungle/State/StateTemplate.h"
#include "Jungle/Test/CaseValueTemplate.h"

namespace traktor::jungle::test
{
	namespace
	{

Ref< const State > transfer(const StateTemplate* st, const State* s)
{
	uint8_t buffer[1024];
	uint32_t size = st->pack(s, buffer, sizeof(buffer));
	return st->unpack(buffer, size);
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.jungle.test.CaseValueTemplate", 0, CaseValueTemplate, traktor::test::Case)

void CaseValueTemplate::run()
{
	{
		StateTemplate st;
		st.declare(new FloatTemplate(L""));

		State s0;
		s0.pack< FloatValue >(0.0f);

		State s1;
		s1.pack< FloatValue >(1.0f);

		State s2;
		s2.pack< FloatValue >(2.0f);

		Ref< const State > S0 = transfer(&st, &s0);
		Ref< const State > S1 = transfer(&st, &s1);
		Ref< const State > S2 = transfer(&st, &s2);

		Ref< const State > r0 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 0.0f);
		Ref< const State > r1 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 0.5f);
		Ref< const State > r2 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 1.0f);
		Ref< const State > r3 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 1.5f);
		Ref< const State > r4 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 2.0f);

		float f0 = r0->getValue< FloatValue >(0);
		CASE_ASSERT_COMPARE(f0, 0.0f, traktor::test::fuzzyEqual);

		float f1 = r1->getValue< FloatValue >(0);
		CASE_ASSERT_COMPARE(f1, 1.0f, traktor::test::fuzzyEqual);

		float f2 = r2->getValue< FloatValue >(0);
		CASE_ASSERT_COMPARE(f2, 2.0f, traktor::test::fuzzyEqual);

		float f3 = r3->getValue< FloatValue >(0);
		CASE_ASSERT_COMPARE(f3, 3.0f, traktor::test::fuzzyEqual);

		float f4 = r4->getValue< FloatValue >(0);
		CASE_ASSERT_COMPARE(f4, 4.0f, traktor::test::fuzzyEqual);
	}

	{
		StateTemplate st;
		st.declare(new FloatTemplate(L"", 1.0f, 0.0f, 2.5f, Ftp8, false));

		State s0;
		s0.pack< FloatValue >(0.0f);

		State s1;
		s1.pack< FloatValue >(1.0f);

		State s2;
		s2.pack< FloatValue >(2.0f);

		Ref< const State > S0 = transfer(&st, &s0);
		Ref< const State > S1 = transfer(&st, &s1);
		Ref< const State > S2 = transfer(&st, &s2);

		Ref< const State > r0 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 0.0f);
		Ref< const State > r1 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 0.5f);
		Ref< const State > r2 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 1.0f);
		Ref< const State > r3 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 1.5f);
		Ref< const State > r4 = st.extrapolate(S0, 0.0f, S1, 0.5f, S2, 1.0f, 2.0f);

		float f0 = r0->getValue< FloatValue >(0);
		CASE_ASSERT_COMPARE(f0, 0.0f, traktor::test::fuzzyEqual);

		float f1 = r1->getValue< FloatValue >(0);
		CASE_ASSERT_COMPARE(f1, 1.0f, traktor::test::fuzzyEqual);

		float f2 = r2->getValue< FloatValue >(0);
		CASE_ASSERT_COMPARE(f2, 2.0f, traktor::test::fuzzyEqual);

		float f3 = r3->getValue< FloatValue >(0);
		CASE_ASSERT_COMPARE(f3, 2.5f, traktor::test::fuzzyEqual);

		float f4 = r4->getValue< FloatValue >(0);
		CASE_ASSERT_COMPARE(f4, 2.5f, traktor::test::fuzzyEqual);
	}
}

}
