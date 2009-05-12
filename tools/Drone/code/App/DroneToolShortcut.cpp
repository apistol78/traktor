#include <Core/System/OS.h>
#include <Core/Io/Path.h>
#include <Core/Serialization/Serializer.h>
#include <Core/Serialization/Member.h>
#include <Ui/MenuItem.h>
#include "App/DroneToolShortcut.h"

namespace traktor
{
	namespace drone
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.drone.DroneToolShortcut", DroneToolShortcut, DroneTool)

DroneToolShortcut::DroneToolShortcut(
	const std::wstring& title,
	const std::wstring& command,
	const std::wstring& commandArguments,
	const std::wstring& workingDirectory
)
:	m_title(title)
,	m_command(command)
,	m_commandArguments(commandArguments)
,	m_workingDirectory(workingDirectory)
{
}

const std::wstring& DroneToolShortcut::getTitle() const
{
	return m_title;
}

const std::wstring& DroneToolShortcut::getCommand() const
{
	return m_command;
}

const std::wstring& DroneToolShortcut::getCommandArguments() const
{
	return m_commandArguments;
}

const std::wstring& DroneToolShortcut::getWorkingDirectory() const
{
	return m_workingDirectory;
}

void DroneToolShortcut::getMenuItems(RefArray< ui::MenuItem >& outItems)
{
	Ref< ui::MenuItem > menuItem = gc_new< ui::MenuItem >(m_title);
	menuItem->setData(L"TOOL", this);
	outItems.push_back(menuItem);
}

bool DroneToolShortcut::execute(ui::Widget* parent, ui::MenuItem* menuItem)
{
	Ref< Process > process = OS::getInstance().execute(m_command, m_commandArguments, m_workingDirectory);
	return process != 0;
}

bool DroneToolShortcut::serialize(Serializer& s)
{
	s >> Member< std::wstring >(L"title", m_title);
	s >> Member< std::wstring >(L"command", m_command);
	s >> Member< std::wstring >(L"commandArguments", m_commandArguments);
	s >> Member< std::wstring >(L"workingDirectory", m_workingDirectory);
	return true;
}

	}
}
