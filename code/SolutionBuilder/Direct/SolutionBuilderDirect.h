/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Containers/SmallSet.h"
#include "Core/Io/Path.h"
#include "Core/Ref.h"
#include "SolutionBuilder/SolutionBuilder.h"

#include <atomic>
#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOLUTIONBUILDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Environment;
class PropertyGroup;

}

namespace traktor::sb
{

class Configuration;
class HeaderScanner;
class Project;
class Solution;

/*! Build solution by invoking compiler, linker and archiver directly.
 * \ingroup SolutionBuilder
 *
 * Settings are sb-direct.xml merged with sb-direct.<platform>.xml, both in resources/build/configurations.
 */
class T_DLLCLASS SolutionBuilderDirect : public SolutionBuilder
{
	T_RTTI_CLASS;

public:
	SolutionBuilderDirect();

	virtual ~SolutionBuilderDirect();

	virtual bool create(const CommandLine& cmdLine) override final;

	virtual bool generate(const Solution* solution, const Path& solutionPathName) override final;

	virtual void showOptions() const override final;

private:
	struct Task;

	/*! Project built in one configuration. */
	struct Unit
	{
		const Solution* solution = nullptr;
		const Project* project = nullptr;
		const Configuration* configuration = nullptr;
		bool enabled = false;
		std::wstring target;
		Path outDir;
		Path intDir;
		Path productPath;
		AlignedVector< std::wstring > includePaths;
		AlignedVector< std::wstring > libraryPaths;
		AlignedVector< std::pair< Unit*, bool > > dependencies; //!< Dependency and if its product should be linked.
		SmallMap< std::wstring, std::wstring > hashes;
		bool hashesModified = false;
		bool rebuild = false;
		bool collected = false;
		AlignedVector< Unit* > collectedLibraries;
		AlignedVector< std::wstring > collectedExternalLibraries;
		AlignedVector< std::wstring > collectedLibraryPaths;
		Task* linkTask = nullptr;
	};

	/*! Single command, executed by a worker. */
	struct Task
	{
		Unit* unit = nullptr;
		std::wstring message;
		std::wstring command;
		std::wstring responseFile;
		std::wstring responseContent;
		std::wstring hash;
		Path output;
		std::wstring echo;
		bool needed = false;
		bool archive = false;
		bool aggregate = false;
		AlignedVector< Task* > dependents;
		int32_t pending = 0;
		bool succeeded = false;
		std::wstring log;
	};

	/*! File in include graph. */
	struct Header
	{
		std::wstring fileName;
		uint64_t timeStamp = 0;
		AlignedVector< int32_t > includes;
		bool scanned = false;
		uint32_t visited = 0;
	};

	Ref< PropertyGroup > m_settings;
	Ref< Environment > m_environment;
	std::wstring m_platform;
	bool m_rebuildAll = false;
	std::wstring m_rebuildProject;
	std::wstring m_target;
	std::wstring m_launch;
	AlignedVector< std::wstring > m_projects;
	bool m_dependentOnly = false;
	bool m_verbose = false;
	SmallMap< std::pair< const Project*, std::wstring >, Unit* > m_units;
	SmallSet< const Solution* > m_solutions;
	AlignedVector< Unit* > m_unitOrder;
	AlignedVector< Task* > m_tasks;
	SmallMap< std::wstring, uint64_t > m_timeStamps;
	SmallMap< std::wstring, std::wstring > m_evaluated;
	Ref< HeaderScanner > m_headerScanner;
	AlignedVector< Header > m_headers;
	SmallMap< std::wstring, int32_t > m_headerIds;
	uint32_t m_visit = 0;
	std::atomic< bool > m_abort;

	Unit* addUnit(const Solution* solution, const Path& solutionPathName, const Project* project, const std::wstring& target);

	bool addSolution(const Solution* solution, const Path& solutionPathName, const std::wstring& target);

	bool planUnit(Unit* unit);

	void collectLibraries(Unit* unit);

	bool execute();

	void executeTask(Task* task);

	bool aggregate(Task* task);

	bool launch();

	std::wstring evaluate(const std::wstring& value);

	uint64_t getTimeStamp(const std::wstring& fileName);

	int32_t getHeader(const std::wstring& fileName);

	uint64_t getIncludedTimeStamp(const std::wstring& fileName);

	std::wstring getOptions(const PropertyGroup* tool, const Unit* unit) const;

	void loadHashes(Unit* unit) const;

	void saveHashes(Unit* unit) const;
};

}
