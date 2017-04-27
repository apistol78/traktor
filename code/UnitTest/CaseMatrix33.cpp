/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "UnitTest/CaseMatrix33.h"
#include "Core/Math/Matrix33.h"

namespace traktor
{

void CaseMatrix33::run()
{
	{
		Matrix33 m1(
			1.0f, 2.0f, 2.0f,
			3.0f, 0.0f, 0.0f,
			3.0f, 0.0f, 0.0f
		);
		Matrix33 m2(
			3.0f, 4.0f, 4.0f,
			5.0f, 0.0f, 0.0f,
			5.0f, 0.0f, 0.0f
		);

		Matrix33 Mc = m1 * m2;
		Matrix33 Mw = m2 * m1;

		/*

		1 * 3 + 2 * 5 + 2 * 5 = 3 + 10 + 10 = 23

		*/

		CASE_ASSERT_EQUAL(Mc.e11, 23.0f);
		CASE_ASSERT_EQUAL(Mc.e[0][0], Mc.e11);
		CASE_ASSERT_EQUAL(Mc.e[1][0], Mc.e21);
		CASE_ASSERT_EQUAL(Mc.e[0][1], Mc.e12);
	}
}

}
