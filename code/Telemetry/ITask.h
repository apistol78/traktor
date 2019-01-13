#pragma once

#include "Core/Object.h"

namespace traktor
{
	namespace telemetry
	{

/*! \brief
 * \ingroup Telemetry
 */
class ITask : public Object
{
	T_RTTI_CLASS;

public:
	enum TaskResult
	{
		TrSuccess = 0,
		TrRetryAgainLater = 1,
		TrFailure = 2
	};

	virtual TaskResult execute() = 0;
};

	}
}
