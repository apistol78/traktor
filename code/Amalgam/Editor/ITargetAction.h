#ifndef traktor_amalgam_ITargetAction_H
#define traktor_amalgam_ITargetAction_H

#include "Core/Object.h"

namespace traktor
{
	namespace amalgam
	{

class ITargetAction : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool execute() = 0;
};

	}
}

#endif	// traktor_amalgam_ITargetAction_H
