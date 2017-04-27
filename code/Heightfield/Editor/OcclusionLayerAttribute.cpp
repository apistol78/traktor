/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Heightfield/Editor/OcclusionLayerAttribute.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.OcclusionLayerAttribute", 0, OcclusionLayerAttribute, world::ILayerAttribute)

OcclusionLayerAttribute::OcclusionLayerAttribute()
:	m_trace(true)
{
}

void OcclusionLayerAttribute::serialize(ISerializer& s)
{
	s >> Member< bool >(L"trace", m_trace);
}

	}
}
