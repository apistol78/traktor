/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderMsvcVCXImportCommon_H
#define SolutionBuilderMsvcVCXImportCommon_H

#include "Core/Serialization/ISerializable.h"

namespace traktor
{

class OutputStream;

	namespace sb
	{

class SolutionBuilderMsvcVCXImportCommon : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual bool generate(OutputStream& os) const = 0;
};

	}
}

#endif	// SolutionBuilderMsvcVCXImportCommon_H
