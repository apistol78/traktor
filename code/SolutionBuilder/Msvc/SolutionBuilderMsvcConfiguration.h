/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderMsvcConfiguration_H
#define SolutionBuilderMsvcConfiguration_H

#include <map>
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{

class OutputStream;

	namespace sb
	{

class SolutionBuilderMsvcTool;
class GeneratorContext;
class Solution;
class Project;
class Configuration;

/*! Visual Studio solution configuration settings. */
class SolutionBuilderMsvcConfiguration : public ISerializable
{
	T_RTTI_CLASS;

public:
	bool generate(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		Configuration* configuration,
		const std::wstring& platform,
		OutputStream& os
	) const;

	virtual void serialize(ISerializer& s);

private:
	std::map< std::wstring, std::wstring > m_staticOptions;
	RefArray< SolutionBuilderMsvcTool > m_tools[2];	//< One array per profile (debug, release).
};

	}
}

#endif	// SolutionBuilderMsvcConfiguration_H
