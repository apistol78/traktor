#include "Scene/Editor/Events/FrameEvent.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.FrameEvent", FrameEvent, ui::Event)

FrameEvent::FrameEvent(ui::EventSubject* sender)
:	ui::Event(sender)
{
}

	}
}