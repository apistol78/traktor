/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderMsvcVCXDefinition_H
#define SolutionBuilderMsvcVCXDefinition_H

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace sb
	{

class GeneratorContext;
class Solution;
class Project;
class ProjectItem;
class Configuration;

class SolutionBuilderMsvcVCXDefinition : public ISerializable
{
	T_RTTI_CLASS;

public:
	SolutionBuilderMsvcVCXDefinition();

	bool generate(
		GeneratorContext& context,
		const Solution* solution,
		const Project* project,
		const Configuration* configuration,
		OutputStream& os
	) const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	struct Option
	{
		std::wstring name;
		std::wstring value;

		void serialize(ISerializer& s);
	};

	std::wstring m_name;
	std::wstring m_fileTypes;
	bool m_resolvePaths;
	std::vector< Option > m_options;

	void collectAdditionalLibraries(
		const Project* project,
		const Configuration* configuration,
		std::set< std::wstring >& outAdditionalLibraries,
		std::set< std::wstring >& outAdditionalLibraryPaths
	) const;

	void findDefinitions(
		GeneratorContext& context,
		const Solution* solution,
		const Project* project,
		const RefArray< ProjectItem >& items
	) const;
};

	}
}

#endif	// SolutionBuilderMsvcVCXDefinition_H
