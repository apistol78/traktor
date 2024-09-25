/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Polar.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Render/SH/SHCoeffs.h"
#include "Render/SH/SHMatrix.h"

namespace traktor::render
{
	namespace
	{

#include "Render/SH/SH.inl"

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.SHCoeffs", 0, SHCoeffs, ISerializable)

void SHCoeffs::resize(int32_t bandCount)
{
	m_bandCount = bandCount;
	m_data.resize(m_bandCount * m_bandCount, Vector4::zero());
}

bool SHCoeffs::empty() const
{
	return m_data.empty();
}

SHCoeffs SHCoeffs::transform(const SHMatrix& matrix) const
{
	SHCoeffs out;
	out.resize(m_data.size());
	for (int32_t r = 0; r < matrix.getRows(); ++r)
	{
		out.m_data[r] = Vector4::zero();
		for (int32_t c = 0; c < matrix.getColumns(); ++c)
			out.m_data[r] += m_data[c] * Scalar(matrix.r(r, c));
	}
	return out;
}

Vector4 SHCoeffs::evaluate(const Polar& direction) const
{
	Vector4 result = Vector4::zero();
	for (int32_t l = 0; l < m_bandCount; ++l)
	{
		for (int32_t m = -l; m <= l; ++m)
		{
			const int32_t index = l * (l + 1) + m;
			result += Scalar(SH(l, m, direction.phi, direction.theta)) * m_data[index];
		}
	}
	return result;
}

Vector4 SHCoeffs::evaluate3(const Polar& direction) const
{
#if !defined(__ANDROID__)
	const Vector4 dir = direction.toUnitCartesian();

	float shd[9];
	shEvaluate3(dir.x(), dir.y(), dir.z(), shd);

	Vector4 result = Vector4::zero();
	result += Scalar(shd[0]) * m_data[0];
	result += Scalar(shd[1]) * m_data[1];
	result += Scalar(shd[2]) * m_data[2];
	result += Scalar(shd[3]) * m_data[3];
	result += Scalar(shd[4]) * m_data[4];
	result += Scalar(shd[5]) * m_data[5];
	result += Scalar(shd[6]) * m_data[6];
	result += Scalar(shd[7]) * m_data[7];
	result += Scalar(shd[8]) * m_data[8];

	return result;
#else
	return Vector4::zero();
#endif
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
	s >> Member< int32_t >(L"bandCount", m_bandCount);
	s >> MemberAlignedVector< Vector4 >(L"data", m_data);
}

}
