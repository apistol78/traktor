#ifndef SolutionBuilderMsvcVCXPropertyGroup_H
#define SolutionBuilderMsvcVCXPropertyGroup_H

#include <map>
#include <Core/Io/OutputStream.h>
#include <Core/Serialization/ISerializable.h>

class GeneratorContext;
class Solution;
class Project;
class ProjectItem;

class SolutionBuilderMsvcVCXPropertyGroup : public traktor::ISerializable
{
	T_RTTI_CLASS;

public:
	bool generate(
		GeneratorContext& context,
		const Solution* solution,
		const Project* project,
		traktor::OutputStream& os
	) const;

	virtual void serialize(traktor::ISerializer& s);

private:
	std::wstring m_label;
	std::wstring m_condition;
	std::map< std::wstring, std::wstring > m_values;
};

#endif	// SolutionBuilderMsvcVCXPropertyGroup_H
