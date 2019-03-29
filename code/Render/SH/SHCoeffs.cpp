#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Render/SH/SHCoeffs.h"
#include "Render/SH/SHMatrix.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.SHCoeffs", 0, SHCoeffs, ISerializable)

void SHCoeffs::resize(size_t coefficientCount)
{
	m_data.resize(coefficientCount, Vector4::zero());
}

bool SHCoeffs::empty() const
{
	return m_data.empty();
}

SHCoeffs SHCoeffs::transform(const SHMatrix& matrix) const
{
	SHCoeffs out;
	out.resize(m_data.size());
	for (int r = 0; r < matrix.getRows(); ++r)
	{
		out.m_data[r] = Vector4::zero();
		for (int c = 0; c < matrix.getColumns(); ++c)
			out.m_data[r] += m_data[c] * Scalar(matrix.r(r, c));
	}
	return out;
}

Vector4 SHCoeffs::operator * (const SHCoeffs& coeffs) const
{
	T_ASSERT(m_data.size() == coeffs.m_data.size());

	Vector4 result = Vector4::zero();
	for (uint32_t i = 0; i < m_data.size(); ++i)
		result += m_data[i] * coeffs.m_data[i];

	return result;
}

void SHCoeffs::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< Vector4 >(L"data", m_data);
}

	}
}
