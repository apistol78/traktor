#ifndef SolutionBuilderMsvcVCXDefinition_H
#define SolutionBuilderMsvcVCXDefinition_H

#include <Core/RefArray.h>
#include <Core/Serialization/ISerializable.h>

class GeneratorContext;
class Solution;
class Project;
class ProjectItem;
class Configuration;

class SolutionBuilderMsvcVCXDefinition : public traktor::ISerializable
{
	T_RTTI_CLASS;

public:
	bool generate(
		GeneratorContext& context,
		const Solution* solution,
		const Project* project,
		const Configuration* configuration,
		traktor::OutputStream& os
	) const;

	virtual void serialize(traktor::ISerializer& s);

private:
	struct Option
	{
		std::wstring name;
		std::wstring value;

		void serialize(traktor::ISerializer& s);
	};

	std::wstring m_name;
	std::wstring m_fileTypes;
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
		const traktor::RefArray< ProjectItem >& items
	) const;
};

#endif	// SolutionBuilderMsvcVCXDefinition_H
