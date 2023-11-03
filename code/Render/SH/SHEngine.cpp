/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cmath>
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Polar.h"
#include "Core/Math/Quasirandom.h"
#include "Core/Math/RandomGeometry.h"
#include "Core/Thread/JobManager.h"
#include "Render/SH/SHEngine.h"
#include "Render/SH/SHFunction.h"
#include "Render/SH/SHMatrix.h"

namespace traktor::render
{
	namespace
	{

#include "Render/SH/SH.inl"

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SHEngine", SHEngine, Object)

SHEngine::SHEngine(uint32_t bandCount)
:	m_bandCount(bandCount)
,	m_coefficientCount(bandCount * bandCount)
{
}

void SHEngine::generateSamplePoints(uint32_t count)
{
	RandomGeometry rg;

	const uint32_t sqrtCount = uint32_t(std::sqrt(double(count)));
	count = sqrtCount * sqrtCount;

	m_samplePoints.resize(count);
	for (uint32_t i = 0; i < sqrtCount; ++i)
	{
		for (uint32_t j = 0; j < sqrtCount; ++j)
		{
			const uint32_t o = i + j * sqrtCount;

			const Vector2 uv = Quasirandom::hammersley(o, count);

			const float phi = 2.0f * std::acos(std::sqrt(1.0f - uv.x));
			const float theta = 2.0f * PI * uv.y;

			m_samplePoints[o].unit = Polar(phi, theta).toUnitCartesian();
			m_samplePoints[o].phi = phi;
			m_samplePoints[o].theta = theta;
			m_samplePoints[o].coefficients.resize(m_coefficientCount);

			for (int32_t l = 0; l < (int32_t)m_bandCount; ++l)
			{
				for (int32_t m = -l; m <= l; ++m)
				{
					const int32_t index = l * (l + 1) + m;
					const float shc = float(SH(l, m, phi, theta));
					m_samplePoints[o].coefficients[index] = Vector4(shc, shc, shc, 0.0f);
				}
			}
		}
	}
}

void SHEngine::generateCoefficients(SHFunction* function, bool parallell, SHCoeffs& outResult)
{
	const double weight = 4.0 * PI;

	const uint32_t sc = uint32_t(m_samplePoints.size() >> 2);

	SHCoeffs intermediate[4];
	intermediate[0].resize(m_bandCount);
	intermediate[1].resize(m_bandCount);
	intermediate[2].resize(m_bandCount);
	intermediate[3].resize(m_bandCount);

	if (parallell)
	{
		const uint32_t nsp = (uint32_t)m_samplePoints.size();

		Job::task_t jobs[4];
		jobs[0] = [&](){ generateCoefficientsJob(function, 0 * sc, 1 * sc, &intermediate[0]); };
		jobs[1] = [&](){ generateCoefficientsJob(function, 1 * sc, 2 * sc, &intermediate[1]); };
		jobs[2] = [&](){ generateCoefficientsJob(function, 2 * sc, 3 * sc, &intermediate[2]); };
		jobs[3] = [&](){ generateCoefficientsJob(function, 3 * sc,    nsp, &intermediate[3]); };
		JobManager::getInstance().fork(jobs, sizeof_array(jobs));
	}
	else
	{
		generateCoefficientsJob(function, 0 * sc, 1 * sc, &intermediate[0]);
		generateCoefficientsJob(function, 1 * sc, 2 * sc, &intermediate[1]);
		generateCoefficientsJob(function, 2 * sc, 3 * sc, &intermediate[2]);
		generateCoefficientsJob(function, 3 * sc, (uint32_t)m_samplePoints.size(), &intermediate[3]);
	}

	outResult.resize(m_bandCount);
	for (uint32_t i = 0; i < m_coefficientCount; ++i)
		outResult[i] = intermediate[0][i] + intermediate[1][i] + intermediate[2][i] + intermediate[3][i];

	Scalar factor(float(weight / m_samplePoints.size()));
	for (uint32_t i = 0; i < m_coefficientCount; ++i)
		outResult[i] *= factor;
}

// SHMatrix SHEngine::generateTransferMatrix(SHFunction* function) const
// {
// 	const double weight = 4.0 * PI;

// 	SHMatrix out(m_coefficientCount, m_coefficientCount);
// 	for (uint32_t ii = 0; ii < m_coefficientCount; ++ii)
// 	{
// 		for (uint32_t jj = 0; jj < m_coefficientCount; ++jj)
// 		{
// 			out.w(ii, jj) = 0.0f;
// 			for (uint32_t s = 0; s < m_samplePoints.size(); ++s)
// 			{
// 				float fs = function->evaluate(m_samplePoints[s].phi, m_samplePoints[s].theta, m_samplePoints[s].unit);
// 				out.w(ii, jj) += fs * m_samplePoints[s].coefficients[ii] * m_samplePoints[s].coefficients[jj];
// 			}
// 			out.w(ii, jj) *= float(weight / m_samplePoints.size());
// 		}
// 	}
// 	return out;
// }

void SHEngine::generateCoefficientsJob(SHFunction* function, uint32_t start, uint32_t end, SHCoeffs* outResult)
{
	for (uint32_t i = start; i < end; ++i)
	{
		const float phi = m_samplePoints[i].phi;
		const float theta = m_samplePoints[i].theta;
		const Vector4 fs = function->evaluate(phi, theta, m_samplePoints[i].unit);
		for (uint32_t n = 0; n < m_coefficientCount; ++n)
			(*outResult)[n] += fs * m_samplePoints[i].coefficients[n];
	}
}

}
