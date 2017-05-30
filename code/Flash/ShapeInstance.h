/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_ShapeInstance_H
#define traktor_flash_ShapeInstance_H

#include "Flash/CharacterInstance.h"

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

/*! \brief Flash shape instance.
 * \ingroup Flash
 */
class T_DLLCLASS ShapeInstance : public CharacterInstance
{
	T_RTTI_CLASS;

public:
	ShapeInstance(ActionContext* context, Dictionary* dictionary, CharacterInstance* parent, const Shape* shape);

	const Shape* getShape() const;

	virtual Aabb2 getBounds() const T_OVERRIDE T_FINAL;

private:
	Ref< const Shape > m_shape;
};

	}
}

#endif	// traktor_flash_ShapeInstance_H
