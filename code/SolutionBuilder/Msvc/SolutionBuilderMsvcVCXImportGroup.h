/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderMsvcVCXImportGroup_H
#define SolutionBuilderMsvcVCXImportGroup_H

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
	SolutionBuilderMsvcVCXImportGroup();

	SolutionBuilderMsvcVCXImportGroup(const std::wstring& label, const std::wstring& condition);

	void addImport(SolutionBuilderMsvcVCXImportCommon* import);

	virtual bool generate(OutputStream& os) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_label;
	std::wstring m_condition;
	RefArray< SolutionBuilderMsvcVCXImportCommon > m_imports;
};

	}
}

#endif	// SolutionBuilderMsvcVCXImportGroup_H
