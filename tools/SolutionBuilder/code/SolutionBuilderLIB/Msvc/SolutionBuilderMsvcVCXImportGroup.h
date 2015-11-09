#ifndef SolutionBuilderMsvcVCXImportGroup_H
#define SolutionBuilderMsvcVCXImportGroup_H

#include <Core/RefArray.h>
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCXImportCommon.h"

class SolutionBuilderMsvcVCXImportGroup : public SolutionBuilderMsvcVCXImportCommon
{
	T_RTTI_CLASS;

public:
	SolutionBuilderMsvcVCXImportGroup();

	SolutionBuilderMsvcVCXImportGroup(const std::wstring& label, const std::wstring& condition);

	void addImport(SolutionBuilderMsvcVCXImportCommon* import);

	virtual bool generate(traktor::OutputStream& os) const;

	virtual void serialize(traktor::ISerializer& s);

private:
	std::wstring m_label;
	std::wstring m_condition;
	traktor::RefArray< SolutionBuilderMsvcVCXImportCommon > m_imports;
};

#endif	// SolutionBuilderMsvcVCXImportGroup_H
