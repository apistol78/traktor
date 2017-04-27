/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_CaseTransform_H
#define traktor_CaseTransform_H

#include "UnitTest/Case.h"

namespace traktor
{

class CaseTransform : public Case
{
public:
	virtual void run() T_OVERRIDE T_FINAL;
};

}

#endif	// traktor_CaseTransform_H
