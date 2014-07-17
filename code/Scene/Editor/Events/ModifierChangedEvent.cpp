#include "Scene/Editor/Events/ModifierChangedEvent.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.ModifierChangedEvent", ModifierChangedEvent, ui::Event)

ModifierChangedEvent::ModifierChangedEvent(ui::EventSubject* sender)
:	ui::Event(sender)
{
}

	}
}