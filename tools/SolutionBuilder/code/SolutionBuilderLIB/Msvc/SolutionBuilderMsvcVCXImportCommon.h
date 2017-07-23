/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderMsvcVCXImportCommon_H
#define SolutionBuilderMsvcVCXImportCommon_H

#include <Core/Io/OutputStream.h>
#include <Core/Serialization/ISerializable.h>

class SolutionBuilderMsvcVCXImportCommon : public traktor::ISerializable
{
	T_RTTI_CLASS;

public:
	virtual bool generate(traktor::OutputStream& os) const = 0;
};

#endif	// SolutionBuilderMsvcVCXImportCommon_H
