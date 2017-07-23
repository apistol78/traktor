/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ddc_ICodeGenerator_H
#define ddc_ICodeGenerator_H

#include <Core/Object.h>
#include <Core/Io/Path.h>

namespace ddc
{

class DfnNode;

class ICodeGenerator : public traktor::Object
{
	T_RTTI_CLASS;

public:
	virtual bool generate(const traktor::Path& sourceFilePath, const DfnNode* node) const = 0;
};

}

#endif	// ddc_ICodeGenerator_H
