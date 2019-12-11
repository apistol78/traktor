#pragma once

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

/*! Integrate modifier persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS IntegrateModifierData : public ModifierData
{
	T_RTTI_CLASS;

public:
	IntegrateModifierData();

	virtual Ref< const Modifier > createModifier(resource::IResourceManager* resourceManager) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	float m_timeScale;
	bool m_linear;
	bool m_angular;
};

	}
}

