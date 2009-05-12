#ifndef traktor_physics_DynamicBodyDesc_H
#define traktor_physics_DynamicBodyDesc_H

#include "Physics/BodyDesc.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*! \brief Dynamic rigid body description.
 * \ingroup Physics
 */
class T_DLLCLASS DynamicBodyDesc : public BodyDesc
{
	T_RTTI_CLASS(DynamicBodyDesc)

public:
	DynamicBodyDesc();

	void setMass(float mass);

	float getMass() const;

	void setAutoDisable(bool autoDisable);

	bool getAutoDisable() const;

	void setDisabled(bool disabled);

	bool getDisabled() const;

	void setLinearDamping(float linearDamping);

	float getLinearDamping() const;

	void setAngularDamping(float angularDamping);

	float getAngularDamping() const;

	void setFriction(float friction);

	float getFriction() const;

	virtual int getVersion() const;

	virtual bool serialize(Serializer& s);

private:
	float m_mass;
	bool m_autoDisable;
	bool m_disabled;
	float m_linearDamping;
	float m_angularDamping;
	float m_friction;
};

	}
}

#endif	// traktor_physics_DynamicBodyDesc_H
