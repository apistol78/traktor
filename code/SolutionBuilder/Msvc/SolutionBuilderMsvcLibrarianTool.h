#pragma once

#include <map>
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcTool.h"

namespace traktor
{
	namespace sb
	{

/*! Visual Studio librarian tool. */
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
	) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::map< std::wstring, std::wstring > m_staticOptions;
};

	}
}

