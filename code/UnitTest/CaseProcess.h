#ifndef traktor_CaseProcess_H
#define traktor_CaseProcess_H

#include "UnitTest/Case.h"

namespace traktor
{

class CaseProcess : public Case
{
public:
	virtual void run() T_OVERRIDE T_FINAL;
};

}

#endif	// traktor_CaseProcess_H
