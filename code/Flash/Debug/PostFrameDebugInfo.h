/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_PostFrameDebugInfo_H
#define traktor_flash_PostFrameDebugInfo_H

#include "Core/RefArray.h"
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
	
class InstanceDebugInfo;

/*! \brief
 * \ingroup Flash
 */
class T_DLLCLASS PostFrameDebugInfo : public ISerializable
{
	T_RTTI_CLASS;

public:
	PostFrameDebugInfo();

	PostFrameDebugInfo(const Aabb2& frameBounds, const RefArray< InstanceDebugInfo >& instances);

	const Aabb2& getFrameBounds() const { return m_frameBounds; }

	const RefArray< InstanceDebugInfo >& getInstances() const { return m_instances; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Aabb2 m_frameBounds;
	RefArray< InstanceDebugInfo > m_instances;
};
	
	}
}

#endif	// traktor_flash_PostFrameDebugInfo_H

