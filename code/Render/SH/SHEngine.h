/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_SHEngine_H
#define traktor_render_SHEngine_H

#include <vector>
#include "Core/Object.h"
#include "Core/Math/Vector4.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/SH/SHCoeffs.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class SHFunction;
class SHMatrix;

/*! \brief Spherical harmonics computation engine.
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

	SHEngine(uint32_t bandCount);

	void generateSamplePoints(uint32_t count);

	void generateCoefficients(SHFunction* function, SHCoeffs& outResult);

	SHMatrix generateTransferMatrix(SHFunction* function) const;

	float evaluate(float phi, float theta, const SHCoeffs& coefficients) const;

private:
	uint32_t m_bandCount;
	uint32_t m_coefficientCount;
	AlignedVector< Sample > m_samplePoints;

	void generateCoefficientsJob(SHFunction* function, uint32_t start, uint32_t end, SHCoeffs* outResult);
};

	}
}

#endif	// traktor_render_SHEngine_H
