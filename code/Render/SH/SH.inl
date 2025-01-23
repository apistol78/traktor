/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

double P(int l, int m, double x)
{
	double pmm = 1.0;
	if (m > 0)
	{
		double somx2 = std::sqrt((1.0 - x) * (1.0 + x));
		double fact = 1.0;
		for (int i = 1; i <= m; ++i)
		{
			pmm *= -fact * somx2;
			fact += 2.0;
		}
	}
	if (l == m)
		return pmm;
	double pmmp1 = x * (2.0 * m + 1.0) * pmm;
	if (l == m + 1)
		return pmmp1;
	double pll = 0.0;
	for (int ll = m + 2; ll <= l; ++ll)
	{
		pll = ((2.0 * ll - 1.0) * x * pmmp1 - (ll + m - 1.0) * pmm) / (ll - m);
		pmm = pmmp1;
		pmmp1 = pll;
	}
	return pll;
}

int factorial(int n)
{
	T_ASSERT(n >= 0);
	int r = 1;
	for (int i = 1; i <= n; ++i)
		r *= i;
	return r;
}

double K(int l, int m)
{
	const double temp = ((2.0 * l + 1.0) * factorial(l - m)) / (4.0 * PI * factorial(l + m));
	return std::sqrt(temp);
}

double SH(int l, int m, double phi, double theta)
{
	const double sqrt2 = std::sqrt(2.0);
	if (m == 0)
		return K(l, 0) * P(l, m, std::cos(phi));
	else if (m > 0)
		return sqrt2 * K(l, m) * std::cos(m * theta) * P(l, m, std::cos(phi));
	else
		return sqrt2 * K(l, -m) * std::sin(-m * theta) * P(l, -m, std::cos(phi));
}

void shEvaluate3(const float fX, const float fY, const float fZ, float* __restrict pSH)
{
	const float fTmpA = -0.48860251190292f;
	const float fTmpB = -1.092548430592079f * fY;
	const float fTmpC = 0.5462742152960395f;

	pSH[0] = 0.2820947917738781f;
	pSH[2] = 0.4886025119029199f * fY;
	pSH[6] = 0.9461746957575601f * fY * fY + -0.3153915652525201f;
	pSH[3] = fTmpA * fX;
	pSH[1] = fTmpA * fZ;
	pSH[7] = fTmpB * fX;
	pSH[5] = fTmpB * fZ;
	pSH[8] = fTmpC * (fX * fX - fZ * fZ);
	pSH[4] = fTmpC * (fX * fZ + fZ * fX);
}
