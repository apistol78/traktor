#ifndef traktor_physics_CollisionSpecification_H
#define traktor_physics_CollisionSpecification_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
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

/*! \brief Collision detection specification.
 * \ingroup Physics
 */
class T_DLLCLASS CollisionSpecification : public ISerializable
{
	T_RTTI_CLASS;

public:
	CollisionSpecification();
	
	explicit CollisionSpecification(uint32_t bitMask);

	uint32_t getBitMask() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	uint32_t m_bitMask;
};

	}
}

#endif	// traktor_physics_CollisionSpecification_H
