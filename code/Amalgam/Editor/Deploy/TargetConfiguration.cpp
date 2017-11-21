/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Amalgam/Editor/Deploy/TargetConfiguration.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.TargetConfiguration", 3, TargetConfiguration, ISerializable)

TargetConfiguration::TargetConfiguration()
{
}

void TargetConfiguration::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& TargetConfiguration::getName() const
{
	return m_name;
}

void TargetConfiguration::setPlatform(const Guid& platform)
{
	m_platform = platform;
}

const Guid& TargetConfiguration::getPlatform() const
{
	return m_platform;
}

void TargetConfiguration::setIcon(const std::wstring& icon)
{
	m_icon = icon;
}

const std::wstring& TargetConfiguration::getIcon() const
{
	return m_icon;
}

void TargetConfiguration::addFeature(const Guid& feature)
{
	m_features.push_back(feature);
}

void TargetConfiguration::removeFeature(const Guid& feature)
{
	std::list< Guid >::iterator i = std::find(m_features.begin(), m_features.end(), feature);
	if (i != m_features.end())
		m_features.erase(i);
}

bool TargetConfiguration::haveFeature(const Guid& feature) const
{
	return std::find(m_features.begin(), m_features.end(), feature) != m_features.end();
}

const std::list< Guid >& TargetConfiguration::getFeatures() const
{
	return m_features;
}

void TargetConfiguration::setRoot(const Guid& root)
{
	m_root = root;
}

const Guid& TargetConfiguration::getRoot() const
{
	return m_root;
}

void TargetConfiguration::setStartup(const Guid& startup)
{
	m_startup = startup;
}

const Guid& TargetConfiguration::getStartup() const
{
	return m_startup;
}

void TargetConfiguration::setDefaultInput(const Guid& defaultInput)
{
	m_defaultInput = defaultInput;
}

const Guid& TargetConfiguration::getDefaultInput() const
{
	return m_defaultInput;
}

void TargetConfiguration::setOnlineConfig(const Guid& onlineConfig)
{
	m_onlineConfig = onlineConfig;
}

const Guid& TargetConfiguration::getOnlineConfig() const
{
	return m_onlineConfig;
}

void TargetConfiguration::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< Guid >(L"platform", m_platform);

	if (s.getVersion< TargetConfiguration >() >= 1 && s.getVersion< TargetConfiguration >() < 3)
	{
		std::wstring systemRoot;
		s >> Member< std::wstring >(L"systemRoot", systemRoot);
	}

	if (s.getVersion< TargetConfiguration >() < 2)
	{
		std::wstring executable;
		s >> Member< std::wstring >(L"executable", executable);
	}

	s >> Member< std::wstring >(L"icon", m_icon);
	s >> MemberStlList< Guid >(L"features", m_features);
	s >> Member< Guid >(L"root", m_root);
	s >> Member< Guid >(L"startup", m_startup);
	s >> Member< Guid >(L"onlineConfig", m_onlineConfig);
	s >> Member< Guid >(L"defaultInput", m_defaultInput);
}

	}
}