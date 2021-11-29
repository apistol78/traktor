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

/*! Brownian motion modifier persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS BrownianModifierData : public ModifierData
{
	T_RTTI_CLASS;

public:
	virtual Ref< const Modifier > createModifier(resource::IResourceManager* resourceManager) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	float m_factor = 0.0f;
};

	}
}

