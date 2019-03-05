#pragma once

#include "Runtime/Types.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace runtime
	{

/*! \brief Reconfigure event.
 * \ingroup Runtime
 *
 * Applications are notified with this
 * event when any server has been reconfigured.
 */
class T_DLLCLASS ReconfigureEvent : public Object
{
	T_RTTI_CLASS;

public:
	ReconfigureEvent(bool finished, int32_t result);

	bool isFinished() const;

	int32_t getResult() const;

private:
	bool m_finished;
	int32_t m_result;
};

	}
}

