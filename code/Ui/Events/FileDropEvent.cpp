#include "Ui/Events/FileDropEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.FileDropEvent", FileDropEvent, Event)

FileDropEvent::FileDropEvent(EventSubject* sender, const std::vector< Path >& files)
:	Event(sender)
,	m_files(files)
{
}

const std::vector< Path >& FileDropEvent::getFiles() const
{
	return m_files;
}

	}
}
