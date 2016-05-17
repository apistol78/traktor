#ifndef traktor_CaseValueTemplate_H
#define traktor_CaseValueTemplate_H

#include "UnitTest/Case.h"

namespace traktor
{

class CaseValueTemplate : public Case
{
public:
	virtual void run() T_OVERRIDE T_FINAL;
};

}

#endif	// traktor_CaseValueTemplate_H
