#include "Scene/Editor/Events/RedrawEvent.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.RedrawEvent", RedrawEvent, ui::Event)

RedrawEvent::RedrawEvent(ui::EventSubject* sender)
:	ui::Event(sender)
{
}

	}
}
