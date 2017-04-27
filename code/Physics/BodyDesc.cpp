/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Physics/BodyDesc.h"
#include "Physics/ShapeDesc.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.BodyDesc", BodyDesc, ISerializable)

void BodyDesc::setShape(ShapeDesc* shape)
{
	m_shape = shape;
}

Ref< const ShapeDesc > BodyDesc::getShape() const
{
	return m_shape;
}

void BodyDesc::serialize(ISerializer& s)
{
	s >> MemberRef< ShapeDesc >(L"shape", m_shape);
}

	}
}
