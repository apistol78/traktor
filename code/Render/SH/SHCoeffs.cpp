#include "Render/SH/SHCoeffs.h"
#include "Render/SH/SHMatrix.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SHCoeffs", SHCoeffs, Object)

void SHCoeffs::resize(uint32_t coefficientCount)
{
	m_coefficients.resize(coefficientCount);
}

bool SHCoeffs::empty() const
{
	return m_coefficients.empty();
}

SHCoeffs SHCoeffs::transform(const SHMatrix& matrix) const
{
	SHCoeffs out;
	out.resize(uint32_t(m_coefficients.size()));
	for (int r = 0; r < matrix.getRows(); ++r)
	{
		out.m_coefficients[r] = 0.0f;
		for (int c = 0; c < matrix.getColumns(); ++c)
			out.m_coefficients[r] += m_coefficients[c] * matrix.r(r, c);
	}
	return out;
}

float SHCoeffs::operator * (const SHCoeffs& coeffs) const
{
	T_ASSERT (m_coefficients.size() == coeffs.m_coefficients.size());

	float result = 0.0f;
	for (uint32_t i = 0; i < m_coefficients.size(); ++i)
		result += m_coefficients[i] * coeffs.m_coefficients[i];

	return result;
}

void SHCoeffs::serialize(ISerializer& s)
{
	s >> MemberStlVector< float >(L"coefficients", m_coefficients);
}

	}
}
