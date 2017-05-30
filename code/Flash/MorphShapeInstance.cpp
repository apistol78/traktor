/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/MorphShape.h"
#include "Flash/MorphShapeInstance.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.MorphShapeInstance", MorphShapeInstance, CharacterInstance)

MorphShapeInstance::MorphShapeInstance(ActionContext* context, Dictionary* dictionary, CharacterInstance* parent, const MorphShape* shape)
:	CharacterInstance(context, "Object", dictionary, parent)
,	m_shape(shape)
{
}

const MorphShape* MorphShapeInstance::getShape() const
{
	return m_shape;
}

Aabb2 MorphShapeInstance::getBounds() const
{
	return getTransform() * m_shape->getShapeBounds();
}

	}
}
