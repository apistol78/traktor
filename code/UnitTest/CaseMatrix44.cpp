/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "UnitTest/CaseMatrix44.h"
#include "UnitTest/MathCompare.h"
#include "Core/Log/Log.h"

namespace traktor
{

void CaseMatrix44::run()
{
	{

		Matrix44 ml(
			1.0f, 2.0f, 2.0f, 2.0f,
			3.0f, 0.0f, 0.0f, 0.0f,
			3.0f, 0.0f, 0.0f, 0.0f,
			3.0f, 0.0f, 0.0f, 0.0f
		);
		Matrix44 mr(
			3.0f, 4.0f, 4.0f, 4.0f,
			5.0f, 0.0f, 0.0f, 0.0f,
			5.0f, 0.0f, 0.0f, 0.0f,
			5.0f, 0.0f, 0.0f, 0.0f
		);

		Matrix44 Mc = ml * mr;
		Matrix44 Mw = mr * ml;

		CASE_ASSERT_EQUAL (Mc(0, 0), 33.0f);
	}

	{
		Matrix44 m(
			1.0f, 2.0f, 3.0f, 4.0f,
			5.0f, 6.0f, 7.0f, 8.0f,
			9.0f, 10.0f, 11.0f, 12.0f,
			13.0f, 14.0f, 15.0f, 16.0f
			);
		Matrix44 mt(
			1.0f, 5.0f, 9.0f, 13.0f,
			2.0f, 6.0f, 10.0f, 14.0f,
			3.0f, 7.0f, 11.0f, 15.0f,
			4.0f, 8.0f, 12.0f, 16.0f
			);

		CASE_ASSERT_COMPARE(m.axisX(),			Vector4(1.0f, 5.0f, 9.0f, 13.0f), compareVectorEqual);
		CASE_ASSERT_COMPARE(m.axisY(),			Vector4(2.0f, 6.0f, 10.0f, 14.0f), compareVectorEqual);
		CASE_ASSERT_COMPARE(m.axisZ(),			Vector4(3.0f, 7.0f, 11.0f, 15.0f), compareVectorEqual);
		CASE_ASSERT_COMPARE(m.translation(),	Vector4(4.0f, 8.0f, 12.0f, 16.0f), compareVectorEqual);

		CASE_ASSERT_COMPARE(m.transpose(), mt, compareMatrixEqual);
	}

	Matrix44 md(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	log::info << L"Determinant = " << md.determinant() << Endl;

	CASE_ASSERT_EQUAL(int32_t(md.determinant()), 1);

	Matrix44 mx = rotateX(PI / 2.0f);
	Matrix44 my = rotateY(PI / 2.0f);
	Matrix44 mz = rotateZ(PI / 2.0f);

	Vector4 vx = mx * Vector4(0.0f, 0.0f, 1.0f, 0.0f);
	Vector4 vy = my * Vector4(1.0f, 0.0f, 0.0f, 0.0f);
	Vector4 vz = mz * Vector4(0.0f, 1.0f, 0.0f, 0.0f);
}

}
