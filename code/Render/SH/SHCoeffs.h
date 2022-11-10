/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class SHMatrix;

/*! Spherical harmonics coefficients.
 * \ingroup Render
 */
class T_DLLCLASS SHCoeffs : public ISerializable
{
	T_RTTI_CLASS;

public:
	void resize(int32_t bandCount);

	bool empty() const;

	SHCoeffs transform(const SHMatrix& matrix) const;

	Vector4 evaluate(float phi, float theta) const;

	Vector4 evaluate3(float phi, float theta) const;

	Vector4 operator * (const SHCoeffs& coeffs) const;

	Vector4& operator [] (uint32_t index) { return m_data[index]; }

	const Vector4& operator [] (uint32_t index) const { return m_data[index]; }

	const AlignedVector< Vector4 >& get() const { return m_data; }

	virtual void serialize(ISerializer& s) override final;

private:
	int32_t m_bandCount;
	AlignedVector< Vector4 > m_data;
};

	}
}
