#ifndef SolutionBuilderMsvcVCXDefinition_H
#define SolutionBuilderMsvcVCXDefinition_H

#include <Core/Serialization/Serializable.h>

class GeneratorContext;
class Solution;
class Project;
class Configuration;

class SolutionBuilderMsvcVCXDefinition : public traktor::Serializable
{
	T_RTTI_CLASS(SolutionBuilderMsvcVCXDefinition)

public:
	bool generate(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		Configuration* configuration,
		traktor::OutputStream& os
	) const;

	virtual bool serialize(traktor::Serializer& s);

private:
	struct Option
	{
		std::wstring name;
		std::wstring value;

		bool serialize(traktor::Serializer& s);
	};

	std::wstring m_name;
	std::wstring m_fileTypes;
	std::vector< Option > m_options;

	void collectAdditionalLibraries(
		Project* project,
		Configuration* configuration,
		std::set< std::wstring >& outAdditionalLibraries,
		std::set< std::wstring >& outAdditionalLibraryPaths
	) const;
};

#endif	// SolutionBuilderMsvcVCXDefinition_H
