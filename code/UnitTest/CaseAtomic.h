#ifndef traktor_CaseAtomic_H
#define traktor_CaseAtomic_H

#include "UnitTest/Case.h"

namespace traktor
{

class CaseAtomic : public Case
{
public:
	virtual void run() T_OVERRIDE T_FINAL;
};

}

#endif	// traktor_CaseAtomic_H
