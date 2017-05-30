/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_ShapeInstanceDebugInfo_H
#define traktor_flash_ShapeInstanceDebugInfo_H

#include "Flash/Debug/InstanceDebugInfo.h"

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

class Shape;
class ShapeInstance;
	
/*! \brief
 * \ingroup Flash
 */
class T_DLLCLASS ShapeInstanceDebugInfo : public InstanceDebugInfo
{
	T_RTTI_CLASS;

public:
	ShapeInstanceDebugInfo();

	ShapeInstanceDebugInfo(const ShapeInstance* instance, bool mask, bool clipped);

	const Shape* getShape() const { return m_shape; }

	bool getMask() const { return m_mask; }

	bool getClipped() const { return m_clipped; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Ref< const Shape > m_shape;
	bool m_mask;
	bool m_clipped;
};
	
	}
}

#endif	// traktor_flash_ShapeInstanceDebugInfo_H

