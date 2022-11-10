/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Math/Vector4.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/SH/SHCoeffs.h"

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

class SHFunction;
class SHMatrix;

/*! Spherical harmonics computation engine.
 * \ingroup Render
 */
class T_DLLCLASS SHEngine : public Object
{
	T_RTTI_CLASS;

public:
	struct Sample
	{
		Vector4 unit;
		float phi;
		float theta;
		SHCoeffs coefficients;
	};

	explicit SHEngine(uint32_t bandCount);

	void generateSamplePoints(uint32_t count);

	void generateCoefficients(SHFunction* function, bool parallell, SHCoeffs& outResult);

	// SHMatrix generateTransferMatrix(SHFunction* function) const;

private:
	uint32_t m_bandCount;
	uint32_t m_coefficientCount;
	AlignedVector< Sample > m_samplePoints;

	void generateCoefficientsJob(SHFunction* function, uint32_t start, uint32_t end, SHCoeffs* outResult);
};

	}
}
