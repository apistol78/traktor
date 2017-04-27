/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "World/EntityData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityData", EntityData, ISerializable)

void EntityData::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& EntityData::getName() const
{
	return m_name;
}

void EntityData::setTransform(const Transform& transform)
{
	m_transform = transform;
}

const Transform& EntityData::getTransform() const
{
	return m_transform;
}

void EntityData::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberComposite< Transform >(L"transform", m_transform);
}

	}
}
