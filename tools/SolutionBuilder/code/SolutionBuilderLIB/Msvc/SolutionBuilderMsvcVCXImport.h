#ifndef SolutionBuilderMsvcVCXImport_H
#define SolutionBuilderMsvcVCXImport_H

#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCXImportCommon.h"

class SolutionBuilderMsvcVCXImport : public SolutionBuilderMsvcVCXImportCommon
{
	T_RTTI_CLASS;

public:
	SolutionBuilderMsvcVCXImport();

	SolutionBuilderMsvcVCXImport(
		const std::wstring& label,
		const std::wstring& project,
		const std::wstring& condition
	);

	virtual bool generate(traktor::OutputStream& os) const;

	virtual void serialize(traktor::ISerializer& s);

private:
	std::wstring m_label;
	std::wstring m_project;
	std::wstring m_condition;
};

#endif	// SolutionBuilderMsvcVCXImport_H
