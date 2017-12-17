#ifndef traktor_physics_Wheel_H
#define traktor_physics_Wheel_H

#include <Core/Ref.h>
#include <Core/Object.h>
#include <Core/Math/Vector4.h>

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class WheelData;

class T_DLLCLASS Wheel : public Object
{
	T_RTTI_CLASS;

public:
	Ref< const WheelData > data;
	float angle;
	float velocity;
	Vector4 direction;
	Vector4 directionPerp;
	float suspensionPreviousLength;
	float suspensionFilteredLength;
	bool contact;
	float contactFudge;
	int32_t contactMaterial;
	Vector4 contactPosition;
	Vector4 contactNormal;
	Vector4 contactVelocity;

	Wheel(const WheelData* data);
};

	}
}

#endif	// traktor_physics_Wheel_H
