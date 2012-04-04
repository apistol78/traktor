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

	void packFloat(float f);

	void packVector3(const Vector4& v);

	void packVector4(const Vector4& v);

	void packQuaternion(const Quaternion& q);

	void packTransform(const Transform& t);

	void unpackBegin();

	float unpackFloat() const;

	Vector4 unpackVector3(float w) const;

	Vector4 unpackVector4() const;

	Quaternion unpackQuaternion() const;

	Transform unpackTransform() const;

	virtual bool verify(const IReplicatableState* targetState) const;

	virtual Ref< IReplicatableState > extrapolate(const IReplicatableState* targetState, float T) const;

	virtual bool serialize(ISerializer& s);

private:
	std::vector< float > m_data;
	mutable uint32_t m_offset;
};

	}
}

#endif	// traktor_parade_PropertyState_H
