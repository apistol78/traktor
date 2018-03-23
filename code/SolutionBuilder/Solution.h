/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef Solution_H
#define Solution_H

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

class Aggregation;
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

	void addAggregation(Aggregation* aggregation);

	void removeAggregation(Aggregation* aggregation);

	const RefArray< Aggregation >& getAggregations() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_name;
	std::wstring m_rootPath;
	std::wstring m_aggregateOutputPath;
	RefArray< Configuration > m_configurations;
	RefArray< Project > m_projects;
	RefArray< Aggregation > m_aggregations;
};

	}
}

#endif	// Solution_H
