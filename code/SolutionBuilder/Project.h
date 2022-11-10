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
class ProjectItem;
class Dependency;

/*! Project settings.
 *
 * Class to describe a project, each project
 * contains several items and configurations defining
 * the project.
 * Also a project can have any number of dependencies
 * to other projects, either to projects in the same solution
 * or to an external project in another solution.
 */
class T_DLLCLASS Project : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setEnable(bool enable);

	bool getEnable() const;

	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	void setSourcePath(const std::wstring& sourcePath);

	const std::wstring& getSourcePath() const;

	/*! \name Configurations. */
	//@{

	void addConfiguration(Configuration* configuration);

	void removeConfiguration(Configuration* configuration);

	const RefArray< Configuration >& getConfigurations() const;

	Configuration* getConfiguration(const std::wstring& name) const;

	//@}

	/*! \name Project items. */
	//@{

	void addItem(ProjectItem* item);

	void removeItem(ProjectItem* item);

	const RefArray< ProjectItem >& getItems() const;

	//@}

	/*! \name Dependencies. */
	//@{

	void addDependency(Dependency* dependency);

	void removeDependency(Dependency* dependency);

	void setDependencies(const RefArray< Dependency >& dependencies);

	const RefArray< Dependency >& getDependencies() const;

	//@}

	virtual void serialize(ISerializer& s) override final;

private:
	bool m_enable = true;
	std::wstring m_name;
	std::wstring m_sourcePath;
	RefArray< Configuration > m_configurations;
	RefArray< ProjectItem > m_items;
	RefArray< Dependency > m_dependencies;
};

	}
}

