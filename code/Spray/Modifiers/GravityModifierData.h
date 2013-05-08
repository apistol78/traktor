#ifndef traktor_spray_GravityModifierData_H
#define traktor_spray_GravityModifierData_H

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

/*! \brief Gravity modifier persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS GravityModifierData : public ModifierData
{
	T_RTTI_CLASS;

public:
	GravityModifierData();

	virtual Ref< Modifier > createModifier(resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s);

private:
	Vector4 m_gravity;
	bool m_world;
};

	}
}

#endif	// traktor_spray_GravityModifierData_H
