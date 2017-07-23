/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef Condition_H
#define Condition_H

#include <vector>
#include <string>
#include <Core/Object.h>
#include <Core/Io/Path.h>
#include <Core/Io/OutputStream.h>

class Source;

class Condition : public traktor::Object
{
	T_RTTI_CLASS;

public:
	virtual void check(const traktor::Path& fileName, const Source& source, bool isHeader, traktor::OutputStream& report) const = 0;
};

#endif	// Condition_H
