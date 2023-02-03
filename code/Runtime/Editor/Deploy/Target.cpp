/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/Editor/Deploy/Target.h"
#include "Runtime/Editor/Deploy/TargetConfiguration.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.runtime.Target", 1, Target, ISerializable)

void Target::setIdentifier(const std::wstring& identifier)
{
	m_identifier = identifier;
}

const std::wstring& Target::getIdentifier() const
{
	return m_identifier;
}

void Target::setVersion(const std::wstring& version)
{
	m_version = version;
}

const std::wstring& Target::getVersion() const
{
	return m_version;
}

void Target::addConfiguration(TargetConfiguration* configuration)
{
	m_configurations.push_back(configuration);
}

void Target::removeConfiguration(TargetConfiguration* configuration)
{
	m_configurations.remove(configuration);
}

void Target::removeAllConfigurations()
{
	m_configurations.resize(0);
}

const RefArray< TargetConfiguration >& Target::getConfigurations() const
{
	return m_configurations;
}

void Target::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"identifier", m_identifier);

	if (s.getVersion< Target >() >= 1)
		 s >> Member< std::wstring >(L"version", m_version);

	s >> MemberRefArray< TargetConfiguration>(L"configurations", m_configurations);
}

}
