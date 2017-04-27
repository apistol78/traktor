/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_telemetry_ITask_H
#define traktor_telemetry_ITask_H

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

#endif	// traktor_telemetry_ITask_H
