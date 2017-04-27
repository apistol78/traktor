/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cmath>
#include "UnitTest/CaseMath.h"
#include "Core/Math/Const.h"
#include "Core/Math/Line2.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Plane.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace
	{

bool compareEqual(const Vector4& a, const Vector4& b)
{
	for (int i = 0; i < 4; ++i)
	{
		if (std::fabs(a[i] - b[i]) > FUZZY_EPSILON)
			return false;
	}
	return true;
}

bool compareNotEqual(const Vector4& a, const Vector4& b)
{
	for (int i = 0; i < 4; ++i)
	{
		if (std::fabs(a[i] - b[i]) > FUZZY_EPSILON)
			return true;
	}
	return false;
}

bool compareFuzzyEqual(float a, float b)
{
	return a >= b - FUZZY_EPSILON && a <= b + FUZZY_EPSILON;
}

	}

void CaseMath::run()
{
	// Line point distance.
	{
		Line2 ln(Vector2(0.0f, 0.0f), Vector2(10.0f, 0.0f));

		float d1 = ln.distance(Vector2(5.0f, -1.0f));
		CASE_ASSERT_COMPARE(d1, 1.0f, compareFuzzyEqual);

		float d2 = ln.distance(Vector2(5.0f, 1.0f));
		CASE_ASSERT_COMPARE(d2, -1.0f, compareFuzzyEqual);
	}

	// Line ray intersection.
	{
		Line2 ln(Vector2(0.0f, 0.0f), Vector2(10.0f, 0.0f));

		float r, k;
		ln.intersect(Ray2(Vector2(2.0f, -1.0f), Vector2(0.0f, 1.0f)), r, k);

		CASE_ASSERT_COMPARE(r, 1.0f, compareFuzzyEqual);
		CASE_ASSERT_COMPARE(k, 2.0f, compareFuzzyEqual);
	}

	Vector2 v1(1.5f, 2.5f);

	CASE_ASSERT_EQUAL(v1.x, 1.5f);
	CASE_ASSERT_EQUAL(v1.y, 2.5f);

	v1.set(3.5f, 4.5f);

	CASE_ASSERT_EQUAL(v1.x, 3.5f);
	CASE_ASSERT_EQUAL(v1.y, 4.5f);

	CASE_ASSERT_EQUAL(v1.length(), std::sqrt(3.5f * 3.5f + 4.5f * 4.5f));
	CASE_ASSERT_EQUAL(v1.length2(), (3.5f * 3.5f + 4.5f * 4.5f));
	
	CASE_ASSERT_EQUAL(Vector2(1.0f, 2.0f) + 3.0f, Vector2(4.0f, 5.0f));
	CASE_ASSERT_EQUAL(Vector2(1.0f, 2.0f) + Vector2(2.0f, 3.0f), Vector2(3.0f, 5.0f));

	// Unaligned store/load
	float vd1[5] = { 666.0f, 1.0f, 2.0f, 3.0f, 4.0f };
	Vector4 unalignedLoad(&vd1[1]);
	CASE_ASSERT_COMPARE(unalignedLoad, Vector4(1.0f, 2.0f, 3.0f, 4.0f), compareEqual);

	float vd2[5] = { 0.0f, 0.0f, 0.0f, 0.0f };
	unalignedLoad.storeUnaligned(&vd2[1]);
	CASE_ASSERT_EQUAL(vd2[0], 0.0f);
	CASE_ASSERT_EQUAL(vd2[1], 1.0f);
	CASE_ASSERT_EQUAL(vd2[2], 2.0f);
	CASE_ASSERT_EQUAL(vd2[3], 3.0f);
	CASE_ASSERT_EQUAL(vd2[4], 4.0f);

	Vector4 v2(1.5f, 2.5f, 3.5f, 4.5f);

	CASE_ASSERT_EQUAL(v2.x(), 1.5f);
	CASE_ASSERT_EQUAL(v2.y(), 2.5f);
	CASE_ASSERT_EQUAL(v2.z(), 3.5f);
	CASE_ASSERT_EQUAL(v2.w(), 4.5f);

	v2.set(5.5f, 6.5f, 7.5f, 8.5f);

	CASE_ASSERT_EQUAL(v2.x(), 5.5f);
	CASE_ASSERT_EQUAL(v2.y(), 6.5f);
	CASE_ASSERT_EQUAL(v2.z(), 7.5f);
	CASE_ASSERT_EQUAL(v2.w(), 8.5f);

	Scalar ln = v2.length();
	float fln = ln;
	float flnr = std::sqrt(5.5f * 5.5f + 6.5f * 6.5f + 7.5f * 7.5f + 8.5f * 8.5f);
	CASE_ASSERT_EQUAL(fln, flnr);

	CASE_ASSERT_EQUAL(v2.length2(), (5.5f * 5.5f + 6.5f * 6.5f + 7.5f * 7.5f + 8.5f * 8.5f));

	CASE_ASSERT_EQUAL(Vector4(1.0f, 2.0f, 3.0f, 4.0f) + Scalar(5.0f), Vector4(6.0f, 7.0f, 8.0f, 9.0f));
	CASE_ASSERT_EQUAL(Vector4(1.0f, 2.0f, 3.0f, 4.0f) + Vector4(5.0f, 6.0f, 7.0f, 8.0f), Vector4(6.0f, 8.0f, 10.0f, 12.0f));

	CASE_ASSERT_EQUAL(rotateX(PI), Quaternion::fromEulerAngles(0.0f, PI, 0.0f).toMatrix44());
	CASE_ASSERT_EQUAL(rotateY(PI), Quaternion::fromEulerAngles(PI, 0.0f, 0.0f).toMatrix44());
	CASE_ASSERT_EQUAL(rotateZ(PI), Quaternion::fromEulerAngles(0.0f, 0.0f, PI).toMatrix44());

	CASE_ASSERT_EQUAL(rotateX(PI), Quaternion(rotateX(PI)).toMatrix44());
	CASE_ASSERT_EQUAL(rotateY(PI), Quaternion(rotateY(PI)).toMatrix44());
	CASE_ASSERT_EQUAL(rotateZ(PI), Quaternion(rotateZ(PI)).toMatrix44());

	{
		Quaternion Q(Vector4(1.0f, 0.0f, 0.0f), Vector4(0.0f, 1.0f, 0.0f));
		Vector4 V(1.0f, 0.0f, 0.0f);
		Vector4 Vr = Q * V;

		CASE_ASSERT_COMPARE(Vr, Vector4(0.0f, 1.0f, 0.0f), compareEqual);
	}

	CASE_ASSERT_COMPARE(Quaternion(Vector4(1.0f, 0.0f, 0.0f), Vector4(0.0f, 1.0f, 0.0f)) * Vector4(1.0f, 0.0f, 0.0f), Vector4(0.0f, 1.0f, 0.0f), compareEqual);
	CASE_ASSERT_COMPARE(Quaternion(Vector4(1.0f, 0.0f, 0.0f), Vector4(0.0f, 0.0f, 1.0f)) * Vector4(1.0f, 0.0f, 0.0f), Vector4(0.0f, 0.0f, 1.0f), compareEqual);
	CASE_ASSERT_COMPARE(Quaternion(Vector4(0.0f, 1.0f, 0.0f), Vector4(0.0f, 0.0f, 1.0f)) * Vector4(0.0f, 1.0f, 0.0f), Vector4(0.0f, 0.0f, 1.0f), compareEqual);

	CASE_ASSERT_COMPARE(rotateZ(PI / 2.0f) * Vector4(1.0f, 0.0f, 0.0f), Vector4(0.0f, 1.0f, 0.0f), compareEqual);
	CASE_ASSERT_COMPARE(rotateY(-PI / 2.0f) * Vector4(1.0f, 0.0f, 0.0f), Vector4(0.0f, 0.0f, 1.0f), compareEqual);
	CASE_ASSERT_COMPARE(rotateX(PI / 2.0f) * Vector4(0.0f, 1.0f, 0.0f), Vector4(0.0f, 0.0f, 1.0f), compareEqual);

	// Rotate translation by 180 deg around Z axis.
	CASE_ASSERT_COMPARE((rotateZ(PI) * translate(1.0f, 0.0f, 0.0f)).translation(), Vector4(-1.0f, 0.0f, 0.0f, 1.0f), compareEqual);

	// Rotate translation by 180 deg around Z axis, incorrect order.
	CASE_ASSERT_COMPARE((translate(1.0f, 0.0f, 0.0f) * rotateZ(PI)).translation(), Vector4(-1.0f, 0.0f, 0.0f, 1.0f), compareNotEqual);

	// First translate point, then rotate 90 deg around Z axis.
	Matrix44 A = translate(1.0f, 0.0f, 0.0f);
	Matrix44 B = rotateZ(PI / 2.0f);
	Matrix44 AB = B * A;

	Vector4 p(2.0f, 0.0f, 0.0f, 1.0f);
	Vector4 pp1 = B * (A * p);
	Vector4 pp2 = AB * p;
	Vector4 pp3 = B * A * p;
	Vector4 pp4 = A * B * p;	// Incorrect order of matrices.

	// Expected result.
	Vector4 r(0.0f, 3.0f, 0.0f, 1.0f);

	CASE_ASSERT_COMPARE(pp1, r, compareEqual);
	CASE_ASSERT_COMPARE(pp1, pp2, compareEqual);
	CASE_ASSERT_COMPARE(pp1, pp3, compareEqual);
	CASE_ASSERT_COMPARE(pp1, pp4, compareNotEqual);
	
	// Plane test.
	Vector4 pt(1.0f, 0.0f, 3.0f, 1.0f);
	
	Plane pl1(Vector4(0.0f, 0.0f, 1.0f), Scalar(-2.0f));
	CASE_ASSERT_COMPARE(pl1.distance(pt), 5.0f, compareFuzzyEqual);
	
	Plane pl2 = translate(0.0f, 0.0f, 1.0f) * pl1;
	CASE_ASSERT_COMPARE(pl2.distance(pt), 4.0f, compareFuzzyEqual);
	
	Plane pl3 = rotateY(deg2rad(90.0f)) * pl1;
	
	Vector4 pt3 = rotateY(deg2rad(90.0f)) * pt;
	Scalar pl3d = pl3.distance(pt);	// 1.0f
	
	CASE_ASSERT_COMPARE(pl3.distance(pt), 3.0f, compareFuzzyEqual);
}

}
