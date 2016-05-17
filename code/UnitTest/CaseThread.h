#ifndef traktor_CaseThread_H
#define traktor_CaseThread_H

#include "UnitTest/Case.h"

namespace traktor
{

class CaseThread : public Case
{
public:
	virtual void run() T_OVERRIDE T_FINAL;
};

}

#endif	// traktor_CaseThread_H
