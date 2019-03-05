#pragma once

#include "Core/Serialization/ISerializable.h"
#include "Core/Timer/Profiler.h"

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

/*! \brief Target profiler events from running target.
 * \ingroup Runtime
 */
class T_DLLCLASS TargetProfilerEvents : public ISerializable
{
	T_RTTI_CLASS;

public:
	TargetProfilerEvents();

	TargetProfilerEvents(double currentTime, const AlignedVector< Profiler::Event >& events);

	double getCurrentTime() const { return m_currentTime; }

	const AlignedVector< Profiler::Event >& getEvents() const { return m_events; }

	virtual void serialize(ISerializer& s) override final;

private:
	double m_currentTime;
	AlignedVector< Profiler::Event > m_events;
};

	}
}

