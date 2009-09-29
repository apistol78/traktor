#ifndef Project_H
#define Project_H

#include <string>
#include <Core/Heap/Ref.h>
#include <Core/Serialization/Serializable.h>

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
class Project : public traktor::Serializable
{
	T_RTTI_CLASS(Project)

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

	traktor::RefList< Configuration >& getConfigurations();

	Configuration* getConfiguration(const std::wstring& name) const;

	//@}

	/*! \name Project items. */
	//@{

	void addItem(ProjectItem* item);

	void removeItem(ProjectItem* item);

	const traktor::RefList< ProjectItem >& getItems() const;

	//@}

	/*! \name Dependencies. */
	//@{

	void addDependency(Dependency* dependency);

	void removeDependency(Dependency* dependency);
	
	void setDependencies(const traktor::RefList< Dependency >& dependencies);

	const traktor::RefList< Dependency >& getDependencies() const;

	//@}

	virtual int getVersion() const;

	virtual bool serialize(traktor::Serializer& s);

private:
	bool m_enable;
	std::wstring m_name;
	std::wstring m_sourcePath;
	traktor::RefList< Configuration > m_configurations;
	traktor::RefList< ProjectItem > m_items;
	traktor::RefList< Dependency > m_dependencies;
};

#endif	// Project_H
