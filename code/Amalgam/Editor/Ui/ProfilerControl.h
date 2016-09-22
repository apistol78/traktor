#ifndef traktor_amalgam_ProfilerControl_H
#define traktor_amalgam_ProfilerControl_H

#include "Amalgam/TargetPerformance.h"
#include "Core/Containers/CircularVector.h"
#include "Ui/Widget.h"

namespace traktor
{
	namespace amalgam
	{

/*! \brief
 * \ingroup Amalgam
 */
class ProfilerControl : public ui::Widget
{
	T_RTTI_CLASS;

public:
	ProfilerControl();

	bool create(ui::Widget* parent);

	void addFrame(float time, const AlignedVector< TargetPerformance::FrameMarker >& markers);

private:
	struct Frame
	{
		float time;
		AlignedVector< TargetPerformance::FrameMarker > markers;
	};

	CircularVector< Frame, 512 > m_frames;
	float m_maxTime;

	void eventPaint(ui::PaintEvent* event);
};

	}
}

#endif	// traktor_amalgam_ProfilerControl_H
