/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_InstanceDebugInfo_H
#define traktor_flash_InstanceDebugInfo_H

#include "Core/Math/Aabb2.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{
	
class FlashCharacterInstance;

/*! \brief
 * \ingroup Flash
 */
class T_DLLCLASS InstanceDebugInfo : public ISerializable
{
	T_RTTI_CLASS;

public:
	InstanceDebugInfo();

	const std::string& getName() const { return m_name; }

	const Aabb2& getBounds() const { return m_bounds; }

	const Matrix33& getTransform() const { return m_transform; }

	virtual void serialize(ISerializer& s) T_OVERRIDE;

protected:
	std::string m_name;
	Aabb2 m_bounds;
	Matrix33 m_transform;
};
	
	}
}

#endif	// traktor_flash_InstanceDebugInfo_H

