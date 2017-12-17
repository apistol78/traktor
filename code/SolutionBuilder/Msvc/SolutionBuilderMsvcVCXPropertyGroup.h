/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderMsvcVCXPropertyGroup_H
#define SolutionBuilderMsvcVCXPropertyGroup_H

#include <map>
#include "Core/Io/OutputStream.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace sb
	{

class GeneratorContext;
class Solution;
class Project;
class ProjectItem;

class SolutionBuilderMsvcVCXPropertyGroup : public ISerializable
{
	T_RTTI_CLASS;

public:
	bool generate(
		GeneratorContext& context,
		const Solution* solution,
		const Project* project,
		OutputStream& os
	) const;

	virtual void serialize(ISerializer& s);

private:
	std::wstring m_label;
	std::wstring m_condition;
	std::map< std::wstring, std::wstring > m_values;
};

	}
}

#endif	// SolutionBuilderMsvcVCXPropertyGroup_H
