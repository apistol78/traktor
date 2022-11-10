/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOLUTIONBUILDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sb
	{

class Configuration;
class Project;

class T_DLLCLASS Solution : public ISerializable
{
	T_RTTI_CLASS;

public:
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

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_name;
	std::wstring m_rootPath;
	std::wstring m_aggregateOutputPath;
	RefArray< Configuration > m_configurations;
	RefArray< Project > m_projects;
};

	}
}

