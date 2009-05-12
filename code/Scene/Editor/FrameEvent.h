#ifndef traktor_scene_FrameEvent_H
#define traktor_scene_FrameEvent_H

#include "Ui/Event.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

class T_DLLCLASS FrameEvent : public ui::Event
{
	T_RTTI_CLASS(FrameEvent)

public:
	FrameEvent(ui::EventSubject* sender, double frameTime, double renderTime);

	double getFrameTime() const;

	double getRenderTime() const;

private:
	double m_frameTime;
	double m_renderTime;
};

	}
}

#endif	// traktor_scene_FrameEvent_H
