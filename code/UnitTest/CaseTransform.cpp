#include "UnitTest/CaseTransform.h"
#include "UnitTest/MathCompare.h"

namespace traktor
{

void CaseTransform::run()
{
	{
		Transform t1(Vector4(-2.0f, 0.0f, 0.0f, 0.0f));
		Transform t2(Vector4(1.0f, 2.0f, 0.0f, 0.0f));

		Matrix44 m1 = translate(-2.0f, 0.0f, 0.0f);
		Matrix44 m2 = translate(1.0f, 2.0f, 0.0f);

		CASE_ASSERT_COMPARE(t1.toMatrix44(), m1, compareMatrixEqual);
		CASE_ASSERT_COMPARE(t2.toMatrix44(), m2, compareMatrixEqual);

		CASE_ASSERT_COMPARE(t1, Transform(m1), compareTransformEqual);
		CASE_ASSERT_COMPARE(t2, Transform(m2), compareTransformEqual);

		Transform T1 = t1 * t2;
		Transform T2 = t2 * t1;

		Matrix44 M1 = m1 * m2;
		Matrix44 M2 = m2 * m1;

		CASE_ASSERT_COMPARE(T1.toMatrix44(), M1, compareMatrixEqual);
		CASE_ASSERT_COMPARE(T2.toMatrix44(), M2, compareMatrixEqual);

		CASE_ASSERT_COMPARE(T1, Transform(M1), compareTransformEqual);
		CASE_ASSERT_COMPARE(T2, Transform(M2), compareTransformEqual);
	}

	{
		const float c_angle = PI / 2.0f;

		Transform t1(Quaternion::fromAxisAngle(Vector4(0.0f, 1.0f, 0.0f, 0.0f), c_angle));
		Transform t2(Quaternion::fromAxisAngle(Vector4(0.0f, 0.0f, 1.0f, 0.0f), c_angle));
		
		Matrix44 m1 = rotateY(c_angle);
		Matrix44 m2 = rotateZ(c_angle);

		CASE_ASSERT_COMPARE(t1.toMatrix44(), m1, compareMatrixEqual);
		CASE_ASSERT_COMPARE(t2.toMatrix44(), m2, compareMatrixEqual);

		CASE_ASSERT_COMPARE(t1, Transform(m1), compareTransformEqual);
		CASE_ASSERT_COMPARE(t2, Transform(m2), compareTransformEqual);

		Transform T1 = t1 * t2;
		Transform T2 = t2 * t1;

		Matrix44 M1 = m1 * m2;
		Matrix44 M2 = m2 * m1;

		CASE_ASSERT_COMPARE(T1.toMatrix44(), M1, compareMatrixEqual);
		CASE_ASSERT_COMPARE(T2.toMatrix44(), M2, compareMatrixEqual);

		CASE_ASSERT_COMPARE(T1, Transform(M1), compareTransformEqual);
		CASE_ASSERT_COMPARE(T2, Transform(M2), compareTransformEqual);
	}

	{
		const float c_angle = PI / 3.0f;

		Transform t1(Vector4(0.0f, 0.0f, 2.0f, 0.0f), Quaternion::fromAxisAngle(Vector4(0.0f, 1.0f, 0.0f, 0.0f), c_angle));
		Transform t2(Vector4(0.0f, 2.0f, 0.0f, 0.0f), Quaternion::fromAxisAngle(Vector4(1.0f, 0.0f, 0.0f, 0.0f), c_angle));

		Matrix44 m1 = translate(0.0f, 0.0f, 2.0f) * rotateY(c_angle);
		Matrix44 m2 = translate(0.0f, 2.0f, 0.0f) * rotateX(c_angle);

		Vector4 P(0.0f, 0.0f, 1.0f, 1.0f);

		Vector4 Pt1 = t1 * P;	// [1 0 2 1]	incorrect [3 0 0 1]
		Vector4 Pt2 = t2 * P;	// [0 1 0 1]

		Vector4 Pm1 = m1 * P;
		Vector4 Pm2 = m2 * P;

		CASE_ASSERT_COMPARE(Pt1, Pm1, compareVectorEqual);
		CASE_ASSERT_COMPARE(Pt2, Pm2, compareVectorEqual);

		Transform t3 = t1 * t2;
		Transform t4 = t2 * t1;

		Matrix44 m3 = m1 * m2;
		Matrix44 m4 = m2 * m1;

		Vector4 Pt3 = t3 * P;
		Vector4 Pm3 = m3 * P;

		CASE_ASSERT_COMPARE(Pt3, Pm3, compareVectorEqual);

		Vector4 Pt4 = t4 * P;
		Vector4 Pm4 = m4 * P;

		CASE_ASSERT_COMPARE(Pt4, Pm4, compareVectorEqual);

		Vector4 PtI = t4.inverse() * Pt4;
		Vector4 PmI = m4.inverse() * Pt4;

		CASE_ASSERT_COMPARE(PtI, P, compareVectorEqual);
		CASE_ASSERT_COMPARE(PmI, P, compareVectorEqual);
	}
}

}
