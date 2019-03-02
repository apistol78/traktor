#pragma once

#include "Core/Object.h"

namespace traktor
{
	namespace online
	{

class TaskQueue;

class ITask : public Object
{
	T_RTTI_CLASS;

public:
	virtual void execute(TaskQueue* taskQueue) = 0;
};

	}
}

