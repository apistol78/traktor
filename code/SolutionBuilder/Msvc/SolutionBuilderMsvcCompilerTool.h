#pragma once

#include <map>
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcTool.h"

namespace traktor
{
	namespace sb
	{

/*! Visual Studio compiler tool. */
class SolutionBuilderMsvcCompilerTool : public SolutionBuilderMsvcTool
{
	T_RTTI_CLASS;

public:
	SolutionBuilderMsvcCompilerTool();

	virtual bool generate(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		Configuration* configuration,
		OutputStream& os
	) const override;

	virtual void serialize(ISerializer& s) override final;

private:
	bool m_resolvePaths;
	std::map< std::wstring, std::wstring > m_staticOptions;
};

	}
}

