#include "Runtime/Editor/Ui/TargetCommandEvent.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.TargetCommandEvent", TargetCommandEvent, ui::Event)

TargetCommandEvent::TargetCommandEvent(ui::EventSubject* sender, TargetInstance* instance, int32_t connectionIndex, const std::wstring& command)
:	Event(sender)
,	m_instance(instance)
,	m_connectionIndex(connectionIndex)
,	m_command(command)
{
}

TargetInstance* TargetCommandEvent::getInstance() const
{
	return m_instance;
}

int32_t TargetCommandEvent::getConnectionIndex() const
{
	return m_connectionIndex;
}

const std::wstring& TargetCommandEvent::getCommand() const
{
	return m_command;
}

	}
}
