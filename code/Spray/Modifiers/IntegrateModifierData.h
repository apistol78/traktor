/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_IntegrateModifierData_H
#define traktor_spray_IntegrateModifierData_H

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

/*! \brief Integrate modifier persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS IntegrateModifierData : public ModifierData
{
	T_RTTI_CLASS;

public:
	IntegrateModifierData();

	virtual Ref< const Modifier > createModifier(resource::IResourceManager* resourceManager) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	float m_timeScale;
	bool m_linear;
	bool m_angular;
};

	}
}

#endif	// traktor_spray_IntegrateModifierData_H
