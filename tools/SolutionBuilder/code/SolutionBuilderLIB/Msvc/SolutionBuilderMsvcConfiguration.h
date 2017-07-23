/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderMsvcConfiguration_H
#define SolutionBuilderMsvcConfiguration_H

#include <map>
#include <Core/RefArray.h>
#include <Core/Serialization/ISerializable.h>
#include <Core/Io/OutputStream.h>

class SolutionBuilderMsvcTool;
class GeneratorContext;
class Solution;
class Project;
class Configuration;

/*! Visual Studio solution configuration settings. */
class SolutionBuilderMsvcConfiguration : public traktor::ISerializable
{
	T_RTTI_CLASS;

public:
	bool generate(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		Configuration* configuration,
		const std::wstring& platform,
		traktor::OutputStream& os
	) const;

	virtual void serialize(traktor::ISerializer& s);

private:
	std::map< std::wstring, std::wstring > m_staticOptions;
	traktor::RefArray< SolutionBuilderMsvcTool > m_tools[2];	//< One array per profile (debug, release).
};

#endif	// SolutionBuilderMsvcConfiguration_H
