/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "UnitTest/CaseQuaternion.h"
#include "UnitTest/MathCompare.h"
#include "Core/Math/Format.h"
#include "Core/Math/RandomGeometry.h"
#include "Core/Log/Log.h"

namespace traktor
{

void CaseQuaternion::run()
{
	{
		RandomGeometry rnd;
		for (int i = 0; i < 10; ++i)
		{
			float angle = rnd.nextFloat() * PI * 2.0f;
			log::info << L"Try " << i << L", angle = " << angle << Endl;

			Quaternion q0 = Quaternion::fromAxisAngle(Vector4(1.0f, 0.0f, 0.0f, 0.0f), angle);
			Quaternion q1 = Quaternion::fromAxisAngle(Vector4(0.0f, 1.0f, 0.0f, 0.0f), angle);
			Quaternion q2 = Quaternion::fromAxisAngle(Vector4(0.0f, 0.0f, 1.0f, 0.0f), angle);

			Matrix44 m0 = rotateX(angle);
			Matrix44 m1 = rotateY(angle);
			Matrix44 m2 = rotateZ(angle);

			CASE_ASSERT_COMPARE(q0.toMatrix44(), m0, compareMatrixEqual);
			CASE_ASSERT_COMPARE(q1.toMatrix44(), m1, compareMatrixEqual);
			CASE_ASSERT_COMPARE(q2.toMatrix44(), m2, compareMatrixEqual);

			CASE_ASSERT_COMPARE(q0, Quaternion(m0), compareQuaternionEqual);
			CASE_ASSERT_COMPARE(q1, Quaternion(m1), compareQuaternionEqual);
			CASE_ASSERT_COMPARE(q2, Quaternion(m2), compareQuaternionEqual);

			Vector4 pt = (rnd.nextUnit() * Scalar(rnd.nextFloat() + 1.0f)).xyz1();
			log::info << L"Rnd point = " << pt << Endl;

			Vector4 m0_pt = m0 * pt;
			Vector4 m1_pt = m1 * pt;
			Vector4 m2_pt = m2 * pt;

			Vector4 q0_pt = q0 * pt;
			Vector4 q1_pt = q1 * pt;
			Vector4 q2_pt = q2 * pt;

			CASE_ASSERT_COMPARE(m0_pt, q0_pt, compareVectorEqual);
			CASE_ASSERT_COMPARE(m1_pt, q1_pt, compareVectorEqual);
			CASE_ASSERT_COMPARE(m2_pt, q2_pt, compareVectorEqual);

			Vector4 q0_pt_p = q0.inverse() * q0_pt;
			Vector4 q1_pt_p = q1.inverse() * q1_pt;
			Vector4 q2_pt_p = q2.inverse() * q2_pt;

			CASE_ASSERT_COMPARE(q0_pt_p, pt, compareVectorEqual);
			CASE_ASSERT_COMPARE(q1_pt_p, pt, compareVectorEqual);
			CASE_ASSERT_COMPARE(q2_pt_p, pt, compareVectorEqual);
		}
	}
}

}
