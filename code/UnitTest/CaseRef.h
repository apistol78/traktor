/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#pragma once

#include "UnitTest/Case.h"

namespace traktor
{

class CaseRef : public Case
{
public:
	virtual void run() T_OVERRIDE T_FINAL;
};

}
