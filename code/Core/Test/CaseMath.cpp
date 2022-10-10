#include <cmath>
#include <functional>
#include "Core/Math/Frustum.h"
#include "Core/Math/Line2.h"
#include "Core/Math/Quasirandom.h"
#include "Core/Math/RandomGeometry.h"
#include "Core/Test/CaseMath.h"
#include "Core/Test/MathCompare.h"

namespace traktor::test
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseMath", 0, CaseMath, Case)

void CaseMath::run()
{
	// Line point distance.
	{
		Line2 ln(Vector2(0.0f, 0.0f), Vector2(10.0f, 0.0f));

		float d1 = ln.distance(Vector2(5.0f, -1.0f));
		CASE_ASSERT_COMPARE(d1, 1.0f, fuzzyEqual);

		float d2 = ln.distance(Vector2(5.0f, 1.0f));
		CASE_ASSERT_COMPARE(d2, -1.0f, fuzzyEqual);
	}

	// Line ray intersection.
	{
		Line2 ln(Vector2(0.0f, 0.0f), Vector2(10.0f, 0.0f));

		float r, k;
		ln.intersect(Ray2(Vector2(2.0f, -1.0f), Vector2(0.0f, 1.0f)), r, k);

		CASE_ASSERT_COMPARE(r, 1.0f, fuzzyEqual);
		CASE_ASSERT_COMPARE(k, 2.0f, fuzzyEqual);
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
	CASE_ASSERT_COMPARE(unalignedLoad, Vector4(1.0f, 2.0f, 3.0f, 4.0f), compareVectorEqual);

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

	//CASE_ASSERT_EQUAL(rotateX(PI), Quaternion(rotateX(PI)).toMatrix44());
	//CASE_ASSERT_EQUAL(rotateY(PI), Quaternion(rotateY(PI)).toMatrix44());
	//CASE_ASSERT_EQUAL(rotateZ(PI), Quaternion(rotateZ(PI)).toMatrix44());

	{
		Quaternion Q(Vector4(1.0f, 0.0f, 0.0f), Vector4(0.0f, 1.0f, 0.0f));
		Vector4 V(1.0f, 0.0f, 0.0f);
		Vector4 Vr = Q * V;

		CASE_ASSERT_COMPARE(Vr, Vector4(0.0f, 1.0f, 0.0f), compareVectorEqual);
	}

	CASE_ASSERT_COMPARE(Quaternion(Vector4(1.0f, 0.0f, 0.0f), Vector4(0.0f, 1.0f, 0.0f)) * Vector4(1.0f, 0.0f, 0.0f), Vector4(0.0f, 1.0f, 0.0f), compareVectorEqual);
	CASE_ASSERT_COMPARE(Quaternion(Vector4(1.0f, 0.0f, 0.0f), Vector4(0.0f, 0.0f, 1.0f)) * Vector4(1.0f, 0.0f, 0.0f), Vector4(0.0f, 0.0f, 1.0f), compareVectorEqual);
	CASE_ASSERT_COMPARE(Quaternion(Vector4(0.0f, 1.0f, 0.0f), Vector4(0.0f, 0.0f, 1.0f)) * Vector4(0.0f, 1.0f, 0.0f), Vector4(0.0f, 0.0f, 1.0f), compareVectorEqual);

	CASE_ASSERT_COMPARE(rotateZ(PI / 2.0f) * Vector4(1.0f, 0.0f, 0.0f), Vector4(0.0f, 1.0f, 0.0f), compareVectorEqual);
	CASE_ASSERT_COMPARE(rotateY(-PI / 2.0f) * Vector4(1.0f, 0.0f, 0.0f), Vector4(0.0f, 0.0f, 1.0f), compareVectorEqual);
	CASE_ASSERT_COMPARE(rotateX(PI / 2.0f) * Vector4(0.0f, 1.0f, 0.0f), Vector4(0.0f, 0.0f, 1.0f), compareVectorEqual);

	// Rotate translation by 180 deg around Z axis.
	CASE_ASSERT_COMPARE((rotateZ(PI) * translate(1.0f, 0.0f, 0.0f)).translation(), Vector4(-1.0f, 0.0f, 0.0f, 1.0f), compareVectorEqual);

	// Rotate translation by 180 deg around Z axis, incorrect order.
	CASE_ASSERT_COMPARE_NOT((translate(1.0f, 0.0f, 0.0f) * rotateZ(PI)).translation(), Vector4(-1.0f, 0.0f, 0.0f, 1.0f), compareVectorEqual);

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

	CASE_ASSERT_COMPARE(pp1, r, compareVectorEqual);
	CASE_ASSERT_COMPARE(pp1, pp2, compareVectorEqual);
	CASE_ASSERT_COMPARE(pp1, pp3, compareVectorEqual);
	CASE_ASSERT_COMPARE_NOT(pp1, pp4, compareVectorEqual);

	// Plane test.
	Vector4 pt(1.0f, 0.0f, 3.0f, 1.0f);

	Plane pl1(Vector4(0.0f, 0.0f, 1.0f), Scalar(-2.0f));
	CASE_ASSERT_COMPARE(pl1.distance(pt), 5.0f, fuzzyEqual);

	Plane pl2 = translate(0.0f, 0.0f, 1.0f) * pl1;
	CASE_ASSERT_COMPARE(pl2.distance(pt), 4.0f, fuzzyEqual);

	Plane pl3 = rotateY(deg2rad(90.0f)) * pl1;
	CASE_ASSERT_COMPARE(pl3.distance(pt), 3.0f, fuzzyEqual);

	// Plane buildFromCorners

	{
		Frustum viewFrustum;
		viewFrustum.buildPerspective(deg2rad(80.0f), 1.0f, 1.0f, 100.0f);
		//viewFrustum.buildOrtho(10.0f, 10.0f, 1.0f, 100.0f);

		const float dx = 1.0f; // / 16.0f;
		const float dy = 1.0f; // / 16.0f;

		Vector4 nh = viewFrustum.corners[1] - viewFrustum.corners[0];
		Vector4 nv = viewFrustum.corners[3] - viewFrustum.corners[0];
		Vector4 fh = viewFrustum.corners[5] - viewFrustum.corners[4];
		Vector4 fv = viewFrustum.corners[7] - viewFrustum.corners[4];

		for (int32_t y = 0; y < 1; ++y)
		{
			float fy = float(y) * dy;
			for (int32_t x = 0; x < 1; ++x)
			{
				float fx = float(x) * dx;

				Vector4 corners[] =
				{
					// Near
					viewFrustum.corners[0] + nh * Scalar(fx) + nv * Scalar(fy),				// l t
					viewFrustum.corners[0] + nh * Scalar(fx + dx) + nv * Scalar(fy),		// r t
					viewFrustum.corners[0] + nh * Scalar(fx + dx) + nv * Scalar(fy + dy),	// r b
					viewFrustum.corners[0] + nh * Scalar(fx) + nv * Scalar(fy + dy),		// l b
					// Far
					viewFrustum.corners[4] + fh * Scalar(fx) + fv * Scalar(fy),				// l t
					viewFrustum.corners[4] + fh * Scalar(fx + dx) + fv * Scalar(fy),		// r t
					viewFrustum.corners[4] + fh * Scalar(fx + dx) + fv * Scalar(fy + dy),	// r b
					viewFrustum.corners[4] + fh * Scalar(fx) + fv * Scalar(fy + dy)			// l b
				};

				Frustum tileFrustum;
				tileFrustum.buildFromCorners(corners);

				CASE_ASSERT(dot3(viewFrustum.planes[0].normal(), tileFrustum.planes[0].normal()) > 0.0f);
				CASE_ASSERT(dot3(viewFrustum.planes[1].normal(), tileFrustum.planes[1].normal()) > 0.0f);
				CASE_ASSERT(dot3(viewFrustum.planes[2].normal(), tileFrustum.planes[2].normal()) > 0.0f);
				CASE_ASSERT(dot3(viewFrustum.planes[3].normal(), tileFrustum.planes[3].normal()) > 0.0f);
				CASE_ASSERT(dot3(viewFrustum.planes[4].normal(), tileFrustum.planes[4].normal()) > 0.0f);
				CASE_ASSERT(dot3(viewFrustum.planes[5].normal(), tileFrustum.planes[5].normal()) > 0.0f);
			}
		}
	}

	// Check quasirandom.
	for (int32_t i = 0; i < 1000; ++i)
	{
		Vector2 uv = Quasirandom::hammersley(i, 1000);
		CASE_ASSERT(uv.x >= 0.0f);
		CASE_ASSERT(uv.x <= 1.0f);
		CASE_ASSERT(uv.y >= 0.0f);
		CASE_ASSERT(uv.y <= 1.0f);
	}

/*
	RandomGeometry rnd;
	for (int32_t i = 0; i < 1000; ++i)
	{
		Vector4 direction = rnd.nextUnit();
		for (int32_t j = 0; j < 1000; ++j)
		{
			Vector2 uv = Quasirandom::hammersley(j, 1000);
			Vector4 rdir = Quasirandom::uniformHemiSphere(uv, direction);

			float ln = rdir.length();
			CASE_ASSERT(ln >= 0.0f);
			CASE_ASSERT(ln <= 1.0f + FUZZY_EPSILON);

			float phi = dot3(rdir, direction);
			CASE_ASSERT_COMPARE(phi, 0.0f, std::greater_equal< float >());
			CASE_ASSERT_COMPARE(phi, 1.0f, std::less_equal< float >());
		}
		for (int32_t j = 0; j < 1000; ++j)
		{
			Vector2 uv = Quasirandom::hammersley(j, 1000);
			Vector4 rdir = Quasirandom::uniformCone(uv, direction, deg2rad(15.0f));
			
			float ln = rdir.length();
			CASE_ASSERT(ln >= 0.0f);
			CASE_ASSERT(ln <= 1.0f + FUZZY_EPSILON);
			
			float phi = dot3(rdir, direction);
			CASE_ASSERT_COMPARE(phi, 0.0f, std::greater_equal< float >());
			CASE_ASSERT_COMPARE(phi, 1.0f + FUZZY_EPSILON, std::less_equal< float >());
		}
		for (int32_t j = 0; j < 1000; ++j)
		{
			Vector2 uv = Quasirandom::hammersley(j, 1000);
			Vector4 rdir = Quasirandom::uniformCone(uv, direction, deg2rad(25.0f));
			
			float ln = rdir.length();
			CASE_ASSERT(ln >= 0.0f);
			CASE_ASSERT(ln <= 1.0f + FUZZY_EPSILON);
			
			float phi = dot3(rdir, direction);
			CASE_ASSERT_COMPARE(phi, 0.0f, std::greater_equal< float >());
			CASE_ASSERT_COMPARE(phi, 1.0f + FUZZY_EPSILON, std::less_equal< float >());
		}
		for (int32_t j = 0; j < 1000; ++j)
		{
			Vector2 uv = Quasirandom::hammersley(j, 1000);
			Vector4 rdir = Quasirandom::uniformCone(uv, direction, deg2rad(45.0f));
			
			float ln = rdir.length();
			CASE_ASSERT(ln >= 0.0f);
			CASE_ASSERT(ln <= 1.0f + FUZZY_EPSILON);
			
			float phi = dot3(rdir, direction);
			CASE_ASSERT_COMPARE(phi, 0.0f, std::greater_equal< float >());
			CASE_ASSERT_COMPARE(phi, 1.0f + FUZZY_EPSILON, std::less_equal< float >());
		}
	}
*/
}

}
