/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderMake_H
#define SolutionBuilderMake_H

#include <map>
#include <set>
#include <string>
#include <Core/Misc/CommandLine.h>
#include "SolutionBuilderLIB/SolutionBuilder.h"

namespace traktor
{

class StreamAdapter;

}

class Project;
class Configuration;

class SolutionBuilderMake : public SolutionBuilder
{
	T_RTTI_CLASS;

public:
	enum Dialect
	{
		MdNMake,	// Microsoft NMake
		MdGnuMake	// GNU make
	};

	enum Platform
	{
		MpWin32,
		MpMacOSX,
		MpLinux,
		MpiOS
	};

	SolutionBuilderMake();

	virtual bool create(const traktor::CommandLine& cmdLine);

	virtual bool generate(Solution* solution);

	virtual void showOptions() const;

private:
	Dialect m_dialect;
	Platform m_platform;
	std::wstring m_config;
	std::wstring m_rootSuffix;
	std::map< std::wstring, std::set< std::wstring > > m_dependencyCache;

	void generateProject(Solution* solution, Project* project);

	void collectLinkDependencies(
		Solution* buildSolution,
		Project* project,
		const std::wstring& configurationName,
		std::set< std::wstring >& outLibraryPaths,
		std::vector< std::wstring >& outLibraryNames
	);

	bool scanDependencies(
		Solution* solution,
		Configuration* configuration,
		const std::wstring& fileName,
		std::set< std::wstring >& outDependencies
	);

	bool getCachedDependencies(const std::wstring& dependencyName, std::set< std::wstring >& outDependencies);

	void addCacheDependencies(const std::wstring& dependencyName, const std::set< std::wstring >& dependencies);

	bool scanDependencies(
		Solution* solution,
		Configuration* configuration,
		const std::wstring& fileName,
		std::set< std::wstring >& visitedDependencies,
		std::set< std::wstring >& resolvedDependencies
	);
};

#endif	// SolutionBuilderMake_H
