/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/SH/SHUtilities.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

float delta(int i, int j)
{
	return i == j ? 1.0f : 0.0f;
}

float u(int l, int m, int n)
{
	if (abs(n) < l)
		return sqrtf(float( ((l + m) * (l - m)) / ((l + n) * (l - n)) ));
	// abs(n) == l
	T_ASSERT (abs(n) == l);
	return sqrtf( ((l + m) * (l - m)) / ((2.0f * l) * (2.0f * l - 1.0f)) );
}

float v(int l, int m, int n)
{
	if (abs(n) < l)
	{
		float ca = (1.0f + delta(m, 0)) * (l + abs(m) - 1.0f) * (l + abs(m));
		float cb = float( (l + n) * (l - n) );
		float cc = (1.0f - 2.0f * delta(m, 0));
		return 0.5f * sqrtf(ca / cb) * cc;
	}
	// abs(n) == l
	T_ASSERT (abs(n) == l);
	float ca = (1.0f + delta(m, 0)) * (l + abs(m) - 1.0f) * (l + abs(m));
	float cb = (2.0f * l) * (2.0f * l - 1.0f);
	float cc = (1.0f - 2.0f * delta(m, 0));
	return 0.5f * sqrtf(ca / cb) * cc;
}

float w(int l, int m, int n)
{
	if (m == 0)
		return 0.0f;
	if (abs(n) < l)
	{
		T_ASSERT ((1.0f - delta(m, 0)) == 1.0f);
		float ca = (l - abs(m) - 1.0f) * (l - abs(m));
		float cb = float( (l + n) * (l - n) );
		return -0.5f * sqrtf(ca / cb);
	}
	// abs(n) == l
	T_ASSERT (abs(n) == l);
	T_ASSERT ((1.0f - delta(m, 0)) == 1.0f);
	float ca = (l - abs(m) - 1.0f) * (l - abs(m));
	float cb = (2.0f * l) * (2.0f * l - 1.0f);
	return -0.5f * sqrtf(ca / cb);
}

float P(const SHMatrix& M, int l, int i, int a, int b)
{
	if (abs(b) < l)
		return M.r(1, i, 0) * M.r(l - 1, a, b);
	else if (b == l)
		return M.r(1, i, 1) * M.r(l - 1, a, l - 1) - M.r(1, i, -1) * M.r(l - 1, a, -l + 1);
	// b == -l
	T_ASSERT (b == -l);
	return M.r(1, i, 1) * M.r(l - 1, a, -l + 1) + M.r(1, i, -1) * M.r(l - 1, a, l - 1);
}

float U(const SHMatrix& M, int l, int m, int n)
{
	return P(M, l, 0, m, n);
}

float V(const SHMatrix& M, int l, int m, int n)
{
	if (m == 0)
		return P(M, l, 1, 1, n) + P(M, l, -1, -1, n);
	else if (m > 0)
		return P(M, l, 1, m - 1, n) * sqrtf(1.0f + delta(m, 1)) - P(M, l, -1, -m + 1, n) * (1.0f - delta(m, 1));
	// m < 0
	T_ASSERT (m < 0);
	return P(M, l, 1, m + 1, n) * (1.0f - delta(m, -1)) + P(M, l, -1, -m - 1, n) * sqrtf(1.0f - delta(m, -1));
}

float W(const SHMatrix& M, int l, int m, int n)
{
	T_ASSERT (m != 0);
	if (m > 0)
		return P(M, l, 1, m + 1, n) + P(M, l, -1, -m - 1, n);
	T_ASSERT (m < 0);
	return P(M, l, 1, m - 1, n) - P(M, l, -1, -m + 1, n);
}

		}

SHMatrix generateRotationSHMatrix(const Matrix44& matrix, int order)
{
	T_ASSERT (order > 0);

	SHMatrix M(order * order, order * order);

	M.w(0, 0, 0) = 1.0f;
	if (order <= 1)
		return M;

	M.w(1, -1, -1) =  matrix(2, 2); M.w(1, -1, 0) = -matrix(1, 2); M.w(1, -1, 1) =  matrix(0, 2);
	M.w(1,  0, -1) = -matrix(2, 1); M.w(1,  0, 0) =  matrix(1, 1); M.w(1,  0, 1) = -matrix(0, 1);
	M.w(1,  1, -1) =  matrix(2, 0); M.w(1,  1, 0) = -matrix(1, 0); M.w(1,  1, 1) =  matrix(0, 0);
	if (order <= 2)
		return M;

	for (int l = 2; l < order; ++l)
	{
		for (int m = -l; m <= l; ++m)
		{
			for (int n = -l; n <= l; ++n)
			{
				float uu = u(l, m, n);
				float vv = v(l, m, n);
				float ww = w(l, m, n);

				float r = 0.0f;

				if (uu != 0.0f)
					r += uu * U(M, l, m, n);
				if (vv != 0.0f)
					r += vv * V(M, l, m, n);
				if (ww != 0.0f)
					r += ww * W(M, l, m, n);

				M.w(l, m, n) = r;
			}
		}
	}

	return M;
}

	}
}
