/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Rational.h"

#include <cmath>
#include <numeric>

namespace traktor
{

// Converts a floating-point value into a rational number using the
// continued-fraction expansion (Stern-Brocot style). The result is the
// best rational approximation whose denominator does not exceed maxDenominator.
Rational floatToRational(double value, int64_t maxDenominator)
{
	// Handle sign separately so the algorithm only deals with positive values.
	const int64_t sign = (value < 0.0) ? -1 : 1;
	value = std::abs(value);

	// h_{n} / k_{n} are the convergents of the continued fraction.
	// We keep the last two convergents at all times.
	int64_t h0 = 0, h1 = 1;
	int64_t k0 = 1, k1 = 0;

	double x = value;
	constexpr int MaxIterations = 64; // continued fractions converge fast

	for (int i = 0; i < MaxIterations; ++i)
	{
		const int64_t a = static_cast< int64_t >(std::floor(x));

		const int64_t h2 = a * h1 + h0;
		const int64_t k2 = a * k1 + k0;

		// If the next convergent would exceed our denominator budget, stop.
		if (k2 > maxDenominator || k2 < 0 /* overflow guard */)
			break;

		h0 = h1;
		h1 = h2;
		k0 = k1;
		k1 = k2;

		const double frac = x - static_cast< double >(a);
		if (frac < 1e-15) // effectively exact
			break;

		x = 1.0 / frac;
	}

	// Reduce to lowest terms (the algorithm already produces a reduced
	// fraction, but std::gcd guards against pathological inputs).
	const int64_t g = std::gcd(h1, k1);
	return Rational{ sign * (h1 / g), k1 / g };
}

}
