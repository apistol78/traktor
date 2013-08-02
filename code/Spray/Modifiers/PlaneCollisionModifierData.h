#ifndef traktor_spray_PlaneCollisionModifierData_H
#define traktor_spray_PlaneCollisionModifierData_H

#include "Core/Math/Plane.h"
#include "Spray/ModifierData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

/*! \brief Plane collision modifier persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS PlaneCollisionModifierData : public ModifierData
{
	T_RTTI_CLASS;

public:
	PlaneCollisionModifierData();

	virtual Ref< Modifier > createModifier(resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s);

private:
	Plane m_plane;
	float m_radius;
	float m_restitution;
};

	}
}

#endif	// traktor_spray_PlaneCollisionModifierData_H
