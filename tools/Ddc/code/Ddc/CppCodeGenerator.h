/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ddc_CppCodeGenerator_H
#define ddc_CppCodeGenerator_H

#include <Core/Io/OutputStream.h>
#include "Ddc/ICodeGenerator.h"

namespace ddc
{

class DfnAlias;

class CppCodeGenerator : public ICodeGenerator
{
	T_RTTI_CLASS;

public:
	virtual bool generate(const traktor::Path& sourceFilePath, const DfnNode* node) const;

private:
	bool generateHeader(const traktor::Path& sourceFilePath, const DfnNode* node) const;

	bool generateHeader(traktor::OutputStream& os, const DfnNode* programNode, const DfnNode* node) const;

	bool generateSource(const traktor::Path& sourceFilePath, const DfnNode* node) const;

	bool generateSource(traktor::OutputStream& os, const DfnNode* programNode, const DfnNode* node) const;
};

}

#endif	// ddc_CppCodeGenerator_H
