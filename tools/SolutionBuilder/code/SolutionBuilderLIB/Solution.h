#ifndef Solution_H
#define Solution_H

#include <string>
#include <Core/Heap/Ref.h>
#include <Core/Serialization/Serializable.h>

class Configuration;
class Project;

class Solution : public traktor::Serializable
{
	T_RTTI_CLASS(Solution)

public:
	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	void setRootPath(const std::wstring& rootPath);

	const std::wstring& getRootPath() const;

	void addProject(Project* project);

	void removeProject(Project* project);

	const traktor::RefList< Project >& getProjects() const;

	virtual bool serialize(traktor::Serializer& s);

private:
	std::wstring m_name;
	std::wstring m_rootPath;
	traktor::RefList< Configuration > m_configurations;
	traktor::RefList< Project > m_projects;
};

#endif	// Solution_H
