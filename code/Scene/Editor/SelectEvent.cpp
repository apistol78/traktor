#include "Scene/Editor/SelectEvent.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.SelectEvent", SelectEvent, ui::Event)

SelectEvent::SelectEvent(ui::EventSubject* sender, const RefArray< EntityAdapter >& selectedEntities)
:	ui::Event(sender, 0)
,	m_selectedEntities(selectedEntities)
{
}

const RefArray< EntityAdapter >& SelectEvent::getSelectedEntities() const
{
	return m_selectedEntities;
}

	}
}