#ifndef traktor_parade_PropertyState_H
#define traktor_parade_PropertyState_H

#include "Core/Math/Transform.h"
#include "Parade/Network/IReplicatableState.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PARADE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace parade
	{

class T_DLLCLASS PropertyState : public IReplicatableState
{
	T_RTTI_CLASS;

public:
	PropertyState();

	void packByte(uint8_t b);

	void packLowPrecision(float f, float min, float max);

	void packFloat(float f);

	void packVector3(const Vector4& v);

	void packVector4(const Vector4& v);

	void packQuaternion(const Quaternion& q);

	void packTransform(const Transform& t);

	void unpackBegin();

	uint8_t unpackByte() const;

	float unpackLowPrecision(float min, float max) const;

	float unpackFloat() const;

	Vector4 unpackVector3(float w) const;

	Vector4 unpackVector4() const;

	Quaternion unpackQuaternion() const;

	Transform unpackTransform() const;

	virtual bool verify(const IReplicatableState* targetState) const;

	virtual Ref< IReplicatableState > extrapolate(
		float T1,
		float T0, const IReplicatableState* S0,
		float T
	) const;

	virtual Ref< IReplicatableState > extrapolate(
		float T2,
		float T1, const IReplicatableState* S1,
		float T0, const IReplicatableState* S0,
		float T
	) const;

	virtual bool serialize(ISerializer& s);

private:
	std::vector< uint8_t > m_u8;
	std::vector< float > m_f32;
	mutable uint32_t m_offsetU8;
	mutable uint32_t m_offsetF32;
};

	}
}

#endif	// traktor_parade_PropertyState_H
