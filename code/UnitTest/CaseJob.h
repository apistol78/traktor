#ifndef traktor_CaseJob_H
#define traktor_CaseJob_H

#include "UnitTest/Case.h"

namespace traktor
{

class CaseJob : public Case
{
public:
	virtual void run() T_OVERRIDE T_FINAL;
};

}

#endif	// traktor_CaseJob_H
