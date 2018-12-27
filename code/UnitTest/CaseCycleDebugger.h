/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_CaseCycleDebugger_H
#define traktor_CaseCycleDebugger_H

#include "UnitTest/Case.h"

namespace traktor
{

class CaseCycleDebugger : public Case
{
public:
	virtual void run() override final;
};

}

#endif	// traktor_CaseCycleDebugger_H
