#include "Scene/Editor/Events/PreModifyEvent.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.PreModifyEvent", PreModifyEvent, ui::Event)

PreModifyEvent::PreModifyEvent(ui::EventSubject* sender)
:	ui::Event(sender)
{
}

	}
}