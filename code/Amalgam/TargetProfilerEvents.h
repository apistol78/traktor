#ifndef traktor_amalgam_TargetProfilerEvents_H
#define traktor_amalgam_TargetProfilerEvents_H

#include "Core/Serialization/ISerializable.h"
#include "Core/Timer/Profiler.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{
	
/*! \brief Target profiler events from running target.
 * \ingroup Amalgam
 */
class T_DLLCLASS TargetProfilerEvents : public ISerializable
{
	T_RTTI_CLASS;

public:
	TargetProfilerEvents();

	TargetProfilerEvents(double currentTime, const AlignedVector< Profiler::Event >& events);

	double getCurrentTime() const { return m_currentTime; }

	const AlignedVector< Profiler::Event >& getEvents() const { return m_events; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	double m_currentTime;
	AlignedVector< Profiler::Event > m_events;
};
	
	}
}

#endif	// traktor_amalgam_TargetProfilerEvents_H
