#ifndef traktor_online_ITask_H
#define traktor_online_ITask_H

#include "Core/Object.h"

namespace traktor
{
	namespace online
	{

class ITask : public Object
{
	T_RTTI_CLASS;

public:
	virtual void execute() = 0;
};

	}
}

#endif	// traktor_online_ITask_H
