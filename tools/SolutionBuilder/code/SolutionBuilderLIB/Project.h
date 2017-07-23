/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef Project_H
#define Project_H

#include <string>
#include <Core/RefArray.h>
#include <Core/Serialization/ISerializable.h>

class Configuration;
class ProjectItem;
class Dependency;

/*! \brief Project settings.
 *
 * Class to describe a project, each project
 * contains several items and configurations defining
 * the project.
 * Also a project can have any number of dependencies
 * to other projects, either to projects in the same solution
 * or to an external project in another solution.
 */
class Project : public traktor::ISerializable
{
	T_RTTI_CLASS;

public:
	Project();

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

	const traktor::RefArray< Configuration >& getConfigurations() const;

	Configuration* getConfiguration(const std::wstring& name) const;

	//@}

	/*! \name Project items. */
	//@{

	void addItem(ProjectItem* item);

	void removeItem(ProjectItem* item);

	const traktor::RefArray< ProjectItem >& getItems() const;

	//@}

	/*! \name Dependencies. */
	//@{

	void addDependency(Dependency* dependency);

	void removeDependency(Dependency* dependency);
	
	void setDependencies(const traktor::RefArray< Dependency >& dependencies);

	const traktor::RefArray< Dependency >& getDependencies() const;

	//@}

	virtual void serialize(traktor::ISerializer& s);

private:
	bool m_enable;
	std::wstring m_name;
	std::wstring m_sourcePath;
	traktor::RefArray< Configuration > m_configurations;
	traktor::RefArray< ProjectItem > m_items;
	traktor::RefArray< Dependency > m_dependencies;
};

#endif	// Project_H
