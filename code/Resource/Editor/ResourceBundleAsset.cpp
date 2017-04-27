/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Resource/Editor/ResourceBundleAsset.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.resource.ResourceBundleAsset", 0, ResourceBundleAsset, ISerializable)

ResourceBundleAsset::ResourceBundleAsset()
:	m_persistent(false)
{
}

const std::vector< Guid >& ResourceBundleAsset::get() const
{
	return m_resources;
}

bool ResourceBundleAsset::persistent() const
{
	return m_persistent;
}

void ResourceBundleAsset::serialize(ISerializer& s)
{
	s >> MemberStlVector< Guid >(L"resources", m_resources);
	s >> Member< bool >(L"persistent", m_persistent);
}

	}
}
