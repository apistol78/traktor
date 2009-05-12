#include <Core/Serialization/Serializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberEnum.h>
#include <Core/Serialization/MemberStl.h>
#include "Configuration.h"

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"Configuration", Configuration, traktor::Serializable)

Configuration::Configuration() :
	m_targetFormat(TfStaticLibrary),
	m_targetProfile(TpDebug)
{
}

void Configuration::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& Configuration::getName() const
{
	return m_name;
}

void Configuration::setTargetFormat(TargetFormat targetFormat)
{
	m_targetFormat = targetFormat;
}

Configuration::TargetFormat Configuration::getTargetFormat() const
{
	return m_targetFormat;
}

void Configuration::setTargetProfile(TargetProfile targetProfile)
{
	m_targetProfile = targetProfile;
}

Configuration::TargetProfile Configuration::getTargetProfile() const
{
	return m_targetProfile;
}

void Configuration::setPrecompiledHeader(const std::wstring& precompiledHeader)
{
	m_precompiledHeader = precompiledHeader;
}

const std::wstring& Configuration::getPrecompiledHeader() const
{
	return m_precompiledHeader;
}

std::vector< std::wstring >& Configuration::getIncludePaths()
{
	return m_includePaths;
}

std::vector< std::wstring >& Configuration::getDefinitions()
{
	return m_definitions;
}

std::vector< std::wstring >& Configuration::getLibraryPaths()
{
	return m_libraryPaths;
}

std::vector< std::wstring >& Configuration::getLibraries()
{
	return m_libraries;
}

bool Configuration::serialize(traktor::Serializer& s)
{
	traktor::MemberEnum< TargetFormat >::Key kTargetFormat[] =
	{
		{ L"TfStaticLibrary", TfStaticLibrary },
		{ L"TfSharedLibrary", TfSharedLibrary },
		{ L"TfExecutable", TfExecutable },
		{ L"TfExecutableConsole", TfExecutableConsole },
		{ 0, 0 }
	};

	traktor::MemberEnum< TargetProfile >::Key kTargetProfile[] =
	{
		{ L"TpDebug", TpDebug },
		{ L"TpRelease", TpRelease },
		{ 0, 0 }
	};

	s >> traktor::Member< std::wstring >(L"name", m_name);
	s >> traktor::MemberEnum< TargetFormat >(L"targetFormat", m_targetFormat, kTargetFormat);
	s >> traktor::MemberEnum< TargetProfile >(L"targetProfile", m_targetProfile, kTargetProfile);
	s >> traktor::Member< std::wstring >(L"precompiledHeader", m_precompiledHeader);
	s >> traktor::MemberStlVector< std::wstring >(L"includePaths", m_includePaths);
	s >> traktor::MemberStlVector< std::wstring >(L"definitions", m_definitions);
	s >> traktor::MemberStlVector< std::wstring >(L"libraryPaths", m_libraryPaths);
	s >> traktor::MemberStlVector< std::wstring >(L"libraries", m_libraries);

	return true;
}
