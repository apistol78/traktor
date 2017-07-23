/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberStl.h>
#include <Core/Serialization/MemberComposite.h>
#include "App/UpdateBundle.h"

namespace traktor
{
	namespace drone
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.drone.UpdateBundle", 0, UpdateBundle, ISerializable)

UpdateBundle::UpdateBundle()
:	m_version(0)
{
}

const std::wstring& UpdateBundle::getDescription() const
{
	return m_description;
}

uint32_t UpdateBundle::getBundleVersion() const
{
	return m_version;
}

const std::vector< UpdateBundle::BundledItem >& UpdateBundle::getItems() const
{
	return m_items;
}

void UpdateBundle::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"version", m_version);
	s >> Member< std::wstring >(L"description", m_description);
	s >> MemberStlVector< BundledItem, MemberComposite< BundledItem > >(L"items", m_items);
}

void UpdateBundle::BundledItem::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"url", url);
	s >> Member< std::wstring >(L"path", path);
}

	}
}
