/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Physics/CompoundShapeDesc.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.CompoundShapeDesc", ShapeDesc::Version, CompoundShapeDesc, ShapeDesc)

void CompoundShapeDesc::serialize(ISerializer& s)
{
	ShapeDesc::serialize(s);
	s >> MemberRefArray< ShapeDesc >(L"shapes", m_shapes);
}

	}
}
