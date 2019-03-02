#pragma once

#include "Core/Containers/AlignedVector.h"
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

/*! \brief Spherical harmonics coefficients.
 * \ingroup Render
 */
class T_DLLCLASS SHCoeffs : public ISerializable
{
	T_RTTI_CLASS;

public:
	void resize(uint32_t coefficientCount);

	bool empty() const;

	SHCoeffs transform(const SHMatrix& matrix) const;

	float operator * (const SHCoeffs& coeffs) const;

	float& operator [] (uint32_t index) { return m_coefficients[index]; }

	float operator [] (uint32_t index) const { return m_coefficients[index]; }

	const AlignedVector< float >& get() const { return m_coefficients; }

	virtual void serialize(ISerializer& s) override final;

private:
	AlignedVector< float > m_coefficients;
};

	}
}
