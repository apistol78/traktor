/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_FlashMorphShapeInstance_H
#define traktor_flash_FlashMorphShapeInstance_H

#include "Flash/FlashCharacterInstance.h"

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

class FlashMorphShape;

/*! \brief Flash morph shape instance.
 * \ingroup Flash
 */
class T_DLLCLASS FlashMorphShapeInstance : public FlashCharacterInstance
{
	T_RTTI_CLASS;

public:
	FlashMorphShapeInstance(ActionContext* context, FlashDictionary* dictionary, FlashCharacterInstance* parent, const FlashMorphShape* shape);

	const FlashMorphShape* getShape() const;

	virtual Aabb2 getBounds() const T_OVERRIDE T_FINAL;

private:
	Ref< const FlashMorphShape > m_shape;
};

	}
}

#endif	// traktor_flash_FlashMorphShapeInstance_H
