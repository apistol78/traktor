#pragma once

#include "Core/RefArray.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXImportCommon.h"

namespace traktor
{
	namespace sb
	{

class SolutionBuilderMsvcVCXImportGroup : public SolutionBuilderMsvcVCXImportCommon
{
	T_RTTI_CLASS;

public:
	SolutionBuilderMsvcVCXImportGroup() = default;

	explicit SolutionBuilderMsvcVCXImportGroup(const std::wstring& label, const std::wstring& condition);

	void addImport(SolutionBuilderMsvcVCXImportCommon* import);

	virtual bool generate(OutputStream& os) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_label;
	std::wstring m_condition;
	RefArray< SolutionBuilderMsvcVCXImportCommon > m_imports;
};

	}
}

