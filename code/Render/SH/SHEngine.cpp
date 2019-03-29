#include "Core/Functor/Functor.h"
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/RandomGeometry.h"
#include "Core/Thread/JobManager.h"
#include "Render/SH/SHEngine.h"
#include "Render/SH/SHFunction.h"
#include "Render/SH/SHMatrix.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

Vector4 polarToCartesian(double phi, double theta)
{
	return Vector4(
		float(cosf(theta) * sinf(phi)),
		float(cosf(phi)),
		float(sinf(theta) * sinf(phi))
	);
}

double P(int l, int m, double x)
{
	double pmm = 1.0;
	if (m > 0)
	{
		double somx2 = sqrtf((1.0 - x) * (1.0 + x));
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
	double temp = ((2.0 * l + 1.0) * factorial(l - m)) / (4.0 * PI * factorial(l + m));
	return sqrtf(temp);
}

double SH(int l, int m, double phi, double theta)
{
	const double sqrt2 = sqrtf(2.0);
	if (m == 0)
		return K(l, 0) * P(l, m, cosf(phi));
	else if (m > 0)
		return sqrt2 * K(l, m) * cosf(m * theta) * P(l, m, cosf(phi));
	else
		return sqrt2 * K(l, -m) * sinf(-m * theta) * P(l, -m, cosf(phi));
}

void shEvaluate3(const float fX, const float fY, const float fZ, float* __restrict pSH)
{
	float fTmpA = -0.48860251190292f;
	float fTmpB = -1.092548430592079f * fY;
	float fTmpC = 0.5462742152960395f;

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

	uint32_t sqrtCount = uint32_t(std::sqrt(double(count)));
	count = sqrtCount * sqrtCount;

	m_samplePoints.resize(count);
	for (uint32_t i = 0; i < sqrtCount; ++i)
	{
		for (uint32_t j = 0; j < sqrtCount; ++j)
		{
			double x = (i + rg.nextDouble()) / double(sqrtCount);
			double y = (j + rg.nextDouble()) / double(sqrtCount);

			double phi = 2.0 * acosf(sqrtf(1.0 - x));
			double theta = 2.0 * PI * y;

			uint32_t o = i + j * sqrtCount;

			m_samplePoints[o].unit = polarToCartesian(phi, theta);
			m_samplePoints[o].phi = float(phi);
			m_samplePoints[o].theta = float(theta);
			m_samplePoints[o].coefficients.resize(m_coefficientCount);

			for (int32_t l = 0; l < int32_t(m_bandCount); ++l)
			{
				for (int32_t m = -l; m <= l; ++m)
				{
					int32_t index = l * (l + 1) + m;
					float shc = float(SH(l, m, phi, theta));
					m_samplePoints[o].coefficients[index] = Vector4(shc, shc, shc, 0.0f);
				}
			}
		}
	}
}

void SHEngine::generateCoefficients(SHFunction* function, SHCoeffs& outResult)
{
	const double weight = 4.0 * PI;

	outResult.resize(m_coefficientCount);
/*
	uint32_t sc = uint32_t(m_samplePoints.size() >> 2);

	RefArray< Functor > jobs(4);
	jobs[0] = makeFunctor(this, &SHEngine::generateCoefficientsJob, function, 0 * sc, 1 * sc, &outResult);
	jobs[1] = makeFunctor(this, &SHEngine::generateCoefficientsJob, function, 1 * sc, 2 * sc, &outResult);
	jobs[2] = makeFunctor(this, &SHEngine::generateCoefficientsJob, function, 2 * sc, 3 * sc, &outResult);
	jobs[3] = makeFunctor(this, &SHEngine::generateCoefficientsJob, function, 3 * sc, 4 * sc, &outResult);
	JobManager::getInstance().fork(jobs);
*/
	generateCoefficientsJob(function, 0, m_samplePoints.size(), &outResult);

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

Vector4 SHEngine::evaluate(float phi, float theta, const SHCoeffs& coefficients) const
{
	Vector4 result = Vector4::zero();
	for (int32_t l = 0; l < int32_t(m_bandCount); ++l)
	{
		for (int32_t m = -l; m <= l; ++m)
		{
			int32_t index = l * (l + 1) + m;
			result += Scalar(SH(l, m, phi, theta)) * coefficients[index];
		}
	}
	return result;
}

Vector4 SHEngine::evaluate3(float phi, float theta, const SHCoeffs& coefficients) const
{
	Vector4 dir(
		cos(theta) * sin(phi),
		cos(phi),
		sin(theta) * sin(phi)
	);

	float shd[9];
	shEvaluate3(dir.x(), dir.y(), dir.z(), shd);

	Vector4 result = Vector4::zero();
	result += Scalar(shd[0]) * coefficients[0];
	result += Scalar(shd[1]) * coefficients[1];
	result += Scalar(shd[2]) * coefficients[2];
	result += Scalar(shd[3]) * coefficients[3];
	result += Scalar(shd[4]) * coefficients[4];
	result += Scalar(shd[5]) * coefficients[5];
	result += Scalar(shd[6]) * coefficients[6];
	result += Scalar(shd[7]) * coefficients[7];
	result += Scalar(shd[8]) * coefficients[8];
	result += Scalar(shd[9]) * coefficients[9];

	return result;
}

void SHEngine::generateCoefficientsJob(SHFunction* function, uint32_t start, uint32_t end, SHCoeffs* outResult)
{
	for (uint32_t i = start; i < end; ++i)
	{
		float phi = m_samplePoints[i].phi;
		float theta = m_samplePoints[i].theta;

		Vector4 fs = function->evaluate(phi, theta, m_samplePoints[i].unit);
		// T_ASSERT(!isNanOrInfinite(fs));
		
		for (uint32_t n = 0; n < m_coefficientCount; ++n)
		{
			(*outResult)[n] += fs * m_samplePoints[i].coefficients[n];
			// T_ASSERT(!isNanOrInfinite((*outResult)[n]));
		}
	}
}

	}
}
