#ifndef traktor_animation_Rotator_H
#define traktor_animation_Rotator_H

#include "Core/Math/Quaternion.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

/* Rotator
 *
 * \brief
 * A rotator is allows for more than 360 degrees of rotation
 * with arbitrary axis order.
 *
 * Rotation is defined using three axis-angle rotations.
 *
 * \ingroup Animation
 */
class T_DLLCLASS Rotator : public ISerializable
{
	T_RTTI_CLASS;

public:
	Rotator();

	Rotator(
		const Vector4& A,
		const Vector4& B,
		const Vector4& C
	);

	Quaternion toQuaternion() const;

	Rotator operator + (const Rotator& rh) const;

	Rotator operator * (const Scalar& rh) const;

	virtual void serialize(ISerializer& s);

private:
	Vector4 m_A;
	Vector4 m_B;
	Vector4 m_C;
};

inline Rotator lerp(const Rotator& a, const Rotator& b, const Scalar& c)
{
	return a * (Scalar(1.0f) - c) + b * c;
}

	}
}

#endif	// traktor_animation_Rotator_H
