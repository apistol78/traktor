#ifndef traktor_CaseSemaphore_H
#define traktor_CaseSemaphore_H

#include "UnitTest/Case.h"

namespace traktor
{

class CaseSemaphore : public Case
{
public:
	virtual void run() T_OVERRIDE T_FINAL;
};

}

#endif	// traktor_CaseSemaphore_H
