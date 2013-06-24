#ifndef SolutionBuilderMsvcGenericTool_H
#define SolutionBuilderMsvcGenericTool_H

#include <map>
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcTool.h"

/*! \brief Visual Studio generic tool. */
class SolutionBuilderMsvcGenericTool : public SolutionBuilderMsvcTool
{
	T_RTTI_CLASS;

public:
	virtual bool generate(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		Configuration* configuration,
		traktor::OutputStream& os
	) const;

	virtual void serialize(traktor::ISerializer& s);

private:
	std::wstring m_toolName;
	std::map< std::wstring, std::wstring > m_staticOptions;
};

#endif	// SolutionBuilderMsvcGenericTool_H
