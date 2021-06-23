#pragma once

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class T_DLLCLASS Material : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setFriction(float friction);

	float getFriction() const;

	void setRestitution(float restitution);

	float getRestitution() const;

	virtual void serialize(ISerializer& s) override final;

private:
	float m_friction = 0.75f;
	float m_restitution = 0.0f;
};

	}
}

