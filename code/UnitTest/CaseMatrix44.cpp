#include "UnitTest/CaseMatrix44.h"
#include "Core/Math/Const.h"
#include "Core/Math/Matrix44.h"
#include "Core/Log/Log.h"

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

bool compareEqual(const Matrix44& a, const Matrix44& b)
{
	for (int i = 0; i < 4; ++i)
	{
		if (!compareEqual(a(i), b(i)))
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

	}

void CaseMatrix44::run()
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

	CASE_ASSERT_COMPARE(m.axisX(), Vector4(1.0f, 2.0f, 3.0f, 4.0f), compareEqual);
	CASE_ASSERT_COMPARE(m.axisY(), Vector4(5.0f, 6.0f, 7.0f, 8.0f), compareEqual);
	CASE_ASSERT_COMPARE(m.axisZ(), Vector4(9.0f, 10.0f, 11.0f, 12.0f), compareEqual);
	CASE_ASSERT_COMPARE(m.translation(), Vector4(13.0f, 14.0f, 15.0f, 16.0f), compareEqual);

	CASE_ASSERT_COMPARE(m.transpose(), mt, compareEqual);

	Matrix44 md(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	log::info << L"Determinant = " << md.determinant() << Endl;

	CASE_ASSERT_EQUAL(int32_t(md.determinant()), 1);

	Matrix44 mdi1 = md.inverse();
	Matrix44 mdi2 = md.inverseOrtho();

	CASE_ASSERT_COMPARE(mdi1, mdi2, compareEqual);
}

}
