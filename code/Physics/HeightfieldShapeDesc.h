#ifndef traktor_physics_HeightfieldShapeDesc_H
#define traktor_physics_HeightfieldShapeDesc_H

#include "Resource/Proxy.h"
#include "Physics/ShapeDesc.h"

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

class Heightfield;

/*! \brief Heightfield collision shape.
 * \ingroup Physics
 */
class T_DLLCLASS HeightfieldShapeDesc : public ShapeDesc
{
	T_RTTI_CLASS;

public:
	void setHeightfield(const resource::Proxy< Heightfield >& heightfield);

	const resource::Proxy< Heightfield >& getHeightfield() const;

	virtual bool bind(resource::IResourceManager* resourceManager);

	virtual bool serialize(ISerializer& s);

private:
	resource::Proxy< Heightfield > m_heightfield;
};

	}
}

#endif	// traktor_physics_HeightfieldShapeDesc_H
