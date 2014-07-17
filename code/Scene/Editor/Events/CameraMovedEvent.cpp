#include "Scene/Editor/Events/CameraMovedEvent.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.CameraMovedEvent", CameraMovedEvent, ui::Event)

CameraMovedEvent::CameraMovedEvent(ui::EventSubject* sender)
:	ui::Event(sender)
{
}

	}
}