/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Test/CaseQuaternion.h"
#include "Core/Test/MathCompare.h"
#include "Core/Math/Polar.h"
#include "Core/Math/RandomGeometry.h"

namespace traktor::test
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseQuaternion", 0, CaseQuaternion, Case)

void CaseQuaternion::run()
{
	// Check to/from euler angles.
	{
		Random rnd;

		for (int32_t i = 0; i < 10; ++i)
		{
			float head = rnd.nextFloat() * TWO_PI;
			float pitch = rnd.nextFloat() * TWO_PI;
			float bank = rnd.nextFloat() * TWO_PI;

			Quaternion q0 = Quaternion::fromEulerAngles(head, pitch, bank);

			float ph, pp, pb;
			q0.toEulerAngles(ph, pp, pb);
			Quaternion q1 = Quaternion::fromEulerAngles(ph, pp, pb);

			for (int32_t t = 0; t <= 10; ++t)
			{
				for (int32_t p = 0; p <= 10; ++p)
				{
					Vector4 unit = Polar((p / 10.0f) * PI, (t / 10.0f) * TWO_PI).toUnitCartesian();

					Vector4 u0 = q0 * unit;
					Vector4 u1 = q1 * unit;

					CASE_ASSERT_COMPARE(u0, u1, compareVectorEqual);
				}
			}
		}
	}

	// Check rotation around each axis.
	{
		RandomGeometry rnd;
		for (int i = 0; i < 10; ++i)
		{
			float angle = rnd.nextFloat() * PI * 2.0f;

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
