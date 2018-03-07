/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderMsvcLibrarianTool_H
#define SolutionBuilderMsvcLibrarianTool_H

#include <map>
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcTool.h"

namespace traktor
{
	namespace sb
	{

/*! \brief Visual Studio librarian tool. */
class SolutionBuilderMsvcLibrarianTool : public SolutionBuilderMsvcTool
{
	T_RTTI_CLASS;

public:
	virtual bool generate(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		Configuration* configuration,
		OutputStream& os
	) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::map< std::wstring, std::wstring > m_staticOptions;
};

	}
}

#endif	// SolutionBuilderMsvcLibrarianTool_H
