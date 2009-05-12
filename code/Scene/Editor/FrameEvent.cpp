#include "Scene/Editor/FrameEvent.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.FrameEvent", FrameEvent, ui::Event)

FrameEvent::FrameEvent(ui::EventSubject* sender, double frameTime, double renderTime)
:	ui::Event(sender, 0)
,	m_frameTime(frameTime)
,	m_renderTime(renderTime)
{
}

double FrameEvent::getFrameTime() const
{
	return m_frameTime;
}

double FrameEvent::getRenderTime() const
{
	return m_renderTime;
}

	}
}