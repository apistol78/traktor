#ifndef traktor_physics_HeightfieldShapeDesc_H
#define traktor_physics_HeightfieldShapeDesc_H

#include "Physics/ShapeDesc.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace hf
	{

class Heightfield;
class MaterialMask;

	}

	namespace physics
	{

/*! \brief Heightfield collision shape.
 * \ingroup Physics
 */
class T_DLLCLASS HeightfieldShapeDesc : public ShapeDesc
{
	T_RTTI_CLASS;

public:
	void setHeightfield(const resource::Proxy< hf::Heightfield >& heightfield);

	const resource::Proxy< hf::Heightfield >& getHeightfield() const;

	void setMaterialMask(const resource::Proxy< hf::MaterialMask >& materialMask);

	const resource::Proxy< hf::MaterialMask >& getMaterialMask() const;

	virtual bool serialize(ISerializer& s);

private:
	resource::Proxy< hf::Heightfield > m_heightfield;
	resource::Proxy< hf::MaterialMask > m_materialMask;
};

	}
}

#endif	// traktor_physics_HeightfieldShapeDesc_H
