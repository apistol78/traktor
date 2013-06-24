#ifndef Solution_H
#define Solution_H

#include <string>
#include <Core/RefArray.h>
#include <Core/Serialization/ISerializable.h>

class Aggregation;
class Configuration;
class Project;

class Solution : public traktor::ISerializable
{
	T_RTTI_CLASS;

public:
	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	void setRootPath(const std::wstring& rootPath);

	const std::wstring& getRootPath() const;

	void addDefinition(const std::wstring& definition);

	void setDefinitions(const std::vector< std::wstring >& definitions);

	const std::vector< std::wstring >& getDefinitions() const;

	void addProject(Project* project);

	void removeProject(Project* project);

	const traktor::RefArray< Project >& getProjects() const;

	void addAggregation(Aggregation* aggregation);

	void removeAggregation(Aggregation* aggregation);

	const traktor::RefArray< Aggregation >& getAggregations() const;

	virtual void serialize(traktor::ISerializer& s);

private:
	std::wstring m_name;
	std::wstring m_rootPath;
	std::vector< std::wstring > m_definitions;
	traktor::RefArray< Configuration > m_configurations;
	traktor::RefArray< Project > m_projects;
	traktor::RefArray< Aggregation > m_aggregations;
};

#endif	// Solution_H
