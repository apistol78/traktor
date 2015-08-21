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
	virtual bool execute() = 0;
};

	}
}

#endif	// traktor_telemetry_ITask_H
