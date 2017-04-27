/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/FlashShapeInstance.h"
#include "Flash/FlashShape.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashShapeInstance", FlashShapeInstance, FlashCharacterInstance)

FlashShapeInstance::FlashShapeInstance(ActionContext* context, FlashDictionary* dictionary, FlashCharacterInstance* parent, const FlashShape* shape)
:	FlashCharacterInstance(context, "Object", dictionary, parent)
,	m_shape(shape)
{
}

const FlashShape* FlashShapeInstance::getShape() const
{
	return m_shape;
}

Aabb2 FlashShapeInstance::getBounds() const
{
	return getTransform() * m_shape->getShapeBounds();
}

	}
}
