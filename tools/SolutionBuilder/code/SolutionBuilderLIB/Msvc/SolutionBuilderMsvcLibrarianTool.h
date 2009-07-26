#ifndef SolutionBuilderMsvcLibrarianTool_H
#define SolutionBuilderMsvcLibrarianTool_H

#include <map>
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcTool.h"

/*! \brief Visual Studio librarian tool. */
class SolutionBuilderMsvcLibrarianTool : public SolutionBuilderMsvcTool
{
	T_RTTI_CLASS(SolutionBuilderMsvcLibrarianTool)

public:
	virtual bool generate(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		Configuration* configuration,
		traktor::OutputStream& os
	) const;

	virtual bool serialize(traktor::Serializer& s);

private:
	std::map< std::wstring, std::wstring > m_staticOptions;
};

#endif	// SolutionBuilderMsvcLibrarianTool_H
