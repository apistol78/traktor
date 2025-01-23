/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Core/Object.h"
#include "Render/SH/SHCoeffs.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
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
		Polar direction;
		SHCoeffs coefficients;
	};

	explicit SHEngine(uint32_t bandCount);

	void generateSamplePoints(uint32_t count);

	void generateCoefficients(const SHFunction* function, bool parallell, SHCoeffs& outResult) const;

	// SHMatrix generateTransferMatrix(const SHFunction* function) const;

private:
	uint32_t m_bandCount;
	uint32_t m_coefficientCount;
	AlignedVector< Sample > m_samplePoints;

	void generateCoefficientsJob(const SHFunction* function, uint32_t start, uint32_t end, SHCoeffs* outResult) const;
};

}
