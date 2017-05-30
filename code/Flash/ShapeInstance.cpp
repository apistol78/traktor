/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/ShapeInstance.h"
#include "Flash/Shape.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ShapeInstance", ShapeInstance, CharacterInstance)

ShapeInstance::ShapeInstance(ActionContext* context, Dictionary* dictionary, CharacterInstance* parent, const Shape* shape)
:	CharacterInstance(context, "Object", dictionary, parent)
,	m_shape(shape)
{
}

const Shape* ShapeInstance::getShape() const
{
	return m_shape;
}

Aabb2 ShapeInstance::getBounds() const
{
	return getTransform() * m_shape->getShapeBounds();
}

	}
}
