/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStl.h"
#include "SolutionBuilder/AggregationItem.h"
#include "SolutionBuilder/Configuration.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"Configuration", 5, Configuration, ISerializable)

Configuration::Configuration()
:	m_targetFormat(TfStaticLibrary)
,	m_targetProfile(TpDebug)
,	m_warningLevel(WlCompilerDefault)
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

void Configuration::addIncludePath(const std::wstring& includePath)
{
	m_includePaths.push_back(includePath);
}

void Configuration::setIncludePaths(const std::vector< std::wstring >& includePaths)
{
	m_includePaths = includePaths;
}

const std::vector< std::wstring >& Configuration::getIncludePaths() const
{
	return m_includePaths;
}

void Configuration::addDefinition(const std::wstring& definition)
{
	m_definitions.push_back(definition);
}

void Configuration::setDefinitions(const std::vector< std::wstring >& definitions)
{
	m_definitions = definitions;
}

const std::vector< std::wstring >& Configuration::getDefinitions() const
{
	return m_definitions;
}

void Configuration::setLibraryPaths(const std::vector< std::wstring >& libraryPaths)
{
	m_libraryPaths = libraryPaths;
}

const std::vector< std::wstring >& Configuration::getLibraryPaths() const
{
	return m_libraryPaths;
}

void Configuration::setLibraries(const std::vector< std::wstring >& libraries)
{
	m_libraries = libraries;
}

const std::vector< std::wstring >& Configuration::getLibraries() const
{
	return m_libraries;
}

void Configuration::setWarningLevel(WarningLevel warningLevel)
{
	m_warningLevel = warningLevel;
}

Configuration::WarningLevel Configuration::getWarningLevel() const
{
	return m_warningLevel;
}

void Configuration::setAdditionalCompilerOptions(const std::wstring& additionalCompilerOptions)
{
	m_additionalCompilerOptions = additionalCompilerOptions;
}

const std::wstring& Configuration::getAdditionalCompilerOptions() const
{
	return m_additionalCompilerOptions;
}

void Configuration::setAdditionalLinkerOptions(const std::wstring& additionalLinkerOptions)
{
	m_additionalLinkerOptions = additionalLinkerOptions;
}

const std::wstring& Configuration::getAdditionalLinkerOptions() const
{
	return m_additionalLinkerOptions;
}

void Configuration::setDebugExecutable(const std::wstring& debugExecutable)
{
	m_debugExecutable = debugExecutable;
}

const std::wstring& Configuration::getDebugExecutable() const
{
	return m_debugExecutable;
}

void Configuration::setDebugArguments(const std::wstring& debugArguments)
{
	m_debugArguments = debugArguments;
}

const std::wstring& Configuration::getDebugArguments() const
{
	return m_debugArguments;
}

void Configuration::setDebugEnvironment(const std::wstring& debugEnvironment)
{
	m_debugEnvironment = debugEnvironment;
}

const std::wstring& Configuration::getDebugEnvironment() const
{
	return m_debugEnvironment;
}

void Configuration::setDebugWorkingDirectory(const std::wstring& debugWorkingDirectory)
{
	m_debugWorkingDirectory = debugWorkingDirectory;
}

const std::wstring& Configuration::getDebugWorkingDirectory() const
{
	return m_debugWorkingDirectory;
}

void Configuration::addAggregationItem(AggregationItem* item)
{
	m_aggregationItems.push_back(item);
}

void Configuration::removeAggregationItem(AggregationItem* item)
{
	m_aggregationItems.remove(item);
}

void Configuration::removeAllAggregationItems()
{
	m_aggregationItems.clear();
}

void Configuration::setAggregationItems(const RefArray< AggregationItem >& aggregationItems)
{
	m_aggregationItems = aggregationItems;
}

const RefArray< AggregationItem >& Configuration::getAggregationItems() const
{
	return m_aggregationItems;
}

void Configuration::setConsumerLibraryPath(const std::wstring& consumerLibraryPath)
{
	m_consumerLibraryPath = consumerLibraryPath;
}

const std::wstring& Configuration::getConsumerLibraryPath() const
{
	return m_consumerLibraryPath;
}

void Configuration::serialize(ISerializer& s)
{
	MemberEnum< TargetFormat >::Key kTargetFormat[] =
	{
		{ L"TfStaticLibrary", TfStaticLibrary },
		{ L"TfSharedLibrary", TfSharedLibrary },
		{ L"TfExecutable", TfExecutable },
		{ L"TfExecutableConsole", TfExecutableConsole },
		{ 0, 0 }
	};

	MemberEnum< TargetProfile >::Key kTargetProfile[] =
	{
		{ L"TpDebug", TpDebug },
		{ L"TpRelease", TpRelease },
		{ 0, 0 }
	};

	MemberEnum< WarningLevel >::Key kWarningLevel[] =
	{
		{ L"WlNoWarnings", WlNoWarnings },
		{ L"WlCriticalOnly", WlCriticalOnly },
		{ L"WlCompilerDefault", WlCompilerDefault },
		{ L"WlAllWarnings", WlAllWarnings },
		{ 0, 0 }
	};

	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberEnum< TargetFormat >(L"targetFormat", m_targetFormat, kTargetFormat);
	s >> MemberEnum< TargetProfile >(L"targetProfile", m_targetProfile, kTargetProfile);
	s >> Member< std::wstring >(L"precompiledHeader", m_precompiledHeader);
	s >> MemberStlVector< std::wstring >(L"includePaths", m_includePaths);
	s >> MemberStlVector< std::wstring >(L"definitions", m_definitions);
	s >> MemberStlVector< std::wstring >(L"libraryPaths", m_libraryPaths);
	s >> MemberStlVector< std::wstring >(L"libraries", m_libraries);

	if (s.getVersion() >= 4)
		s >> MemberEnum< WarningLevel >(L"warningLevel", m_warningLevel, kWarningLevel);

	if (s.getVersion() >= 1)
	{
		s >> Member< std::wstring >(L"additionalCompilerOptions", m_additionalCompilerOptions);
		s >> Member< std::wstring >(L"additionalLinkerOptions", m_additionalLinkerOptions);
	}

	if (s.getVersion() >= 3)
	{
		s >> Member< std::wstring >(L"debugExecutable", m_debugExecutable);
		s >> Member< std::wstring >(L"debugArguments", m_debugArguments);
		s >> Member< std::wstring >(L"debugEnvironment", m_debugEnvironment);
		s >> Member< std::wstring >(L"debugWorkingDirectory", m_debugWorkingDirectory);
	}

	if (s.getVersion() >= 2)
		s >> MemberRefArray< AggregationItem >(L"aggregationItems", m_aggregationItems);

	if (s.getVersion() >= 5)
		s >> Member< std::wstring >(L"consumerLibraryPath", m_consumerLibraryPath);
}

	}
}
