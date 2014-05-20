#ifndef traktor_amalgam_FrameProfiler_H
#define traktor_amalgam_FrameProfiler_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Timer/Timer.h"

namespace traktor
{
	namespace amalgam
	{

/*! \brief
 * \ingroup Amalgam
 */
enum FrameProfilerTags
{
	FptEndFrame,
	FptRenderServerUpdate,
	FptSessionManagerUpdate,
	FptScriptGC,
	FptAudioServerUpdate,
	FptRumbleUpdate,
	FptInputServerUpdate,
	FptStateUpdate,
	FptPhysicsServerUpdate,
	FptBuildFrame,
	FptAudioLayer,
	FptFlashLayer,
	FptVideoLayer,
	FptWorldLayer,
	FptWorldLayerEvents,
	FptScript
};

/*! \brief
 * \ingroup Amalgam
 */
class FrameProfiler
{
public:
	struct Marker
	{
		uint8_t id;
		uint8_t level;
		double begin;
		double end;
	};

	void beginFrame();

	void endFrame();

	void beginScope(uint8_t id);

	void endScope();

	const AlignedVector< Marker >& getMarkers() const { return m_markers; }

private:
	Timer m_timer;
	AlignedVector< Marker > m_markers;
	AlignedVector< int32_t > m_stack;
};

	}
}

#endif	// traktor_amalgam_FrameProfiler_H
