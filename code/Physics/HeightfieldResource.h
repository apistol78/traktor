#ifndef traktor_physics_HeightfieldResource_H
#define traktor_physics_HeightfieldResource_H

#include "Core/Serialization/Serializable.h"
#include "Core/Math/Vector4.h"

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

/*! \brief Heightfield resource.
 * \ingroup Physics
 */
class T_DLLCLASS HeightfieldResource : public Serializable
{
	T_RTTI_CLASS(HeightfieldResource)

public:
	HeightfieldResource(uint32_t size = 0, const Vector4& worldExtent = Vector4::zero());

	uint32_t getSize() const;

	const Vector4& getWorldExtent() const;

	virtual bool serialize(Serializer& s);

private:
	uint32_t m_size;
	Vector4 m_worldExtent;
};

	}
}

#endif	// traktor_physics_HeightfieldResource_H
