/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Containers/AlignedVector.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOLUTIONBUILDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sb
{

class Configuration;
class Project;

class T_DLLCLASS Solution : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! Environment variable set into builder process before solution is generated. */
	struct EnvironmentVariable
	{
		std::wstring name;
		std::wstring value;
		bool overwrite = true; //!< Replace variable if already set.

		void serialize(ISerializer& s);
	};

	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	void setRootPath(const std::wstring& rootPath);

	const std::wstring& getRootPath() const;

	void setAggregateOutputPath(const std::wstring& aggregateOutputPath);

	const std::wstring& getAggregateOutputPath() const;

	void addProject(Project* project);

	void removeProject(Project* project);

	void setProjects(const RefArray< Project >& projects);

	const RefArray< Project >& getProjects() const;

	void setStartupProject(Project* project);

	Project* getStartupProject() const;

	void setEnvironmentVariables(const AlignedVector< EnvironmentVariable >& environmentVariables);

	const AlignedVector< EnvironmentVariable >& getEnvironmentVariables() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_name;
	std::wstring m_rootPath;
	std::wstring m_aggregateOutputPath;
	RefArray< Configuration > m_configurations;
	RefArray< Project > m_projects;
	Ref< Project > m_startupProject;
	AlignedVector< EnvironmentVariable > m_environmentVariables;
};

}
