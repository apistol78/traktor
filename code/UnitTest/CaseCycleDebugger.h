#ifndef traktor_CaseCycleDebugger_H
#define traktor_CaseCycleDebugger_H

#include "UnitTest/Case.h"

namespace traktor
{

class CaseCycleDebugger : public Case
{
public:
	virtual void run() T_OVERRIDE T_FINAL;
};

}

#endif	// traktor_CaseCycleDebugger_H
