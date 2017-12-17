/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/System/OS.h>
#include <Core/Io/Path.h>
#include <Core/Serialization/Serializer.h>
#include <Core/Serialization/Member.h>
#include <Ui/MenuItem.h>
#include "Drone/App/DroneToolShortcut.h"

namespace traktor
{
	namespace drone
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.drone.DroneToolShortcut", 0, DroneToolShortcut, DroneTool)

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
	Ref< ui::MenuItem > menuItem = new ui::MenuItem(m_title);
	menuItem->setData(L"TOOL", this);
	outItems.push_back(menuItem);
}

bool DroneToolShortcut::execute(ui::Widget* parent, ui::MenuItem* menuItem)
{
	std::wstring commandLine = m_command;
	if (!m_commandArguments.empty())
		commandLine = commandLine + L" " + m_commandArguments;

	Ref< IProcess > process = OS::getInstance().execute(
		commandLine,
		m_workingDirectory,
		0,
		false,
		false,
		false
	);
	return process != 0;
}

void DroneToolShortcut::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"title", m_title);
	s >> Member< std::wstring >(L"command", m_command);
	s >> Member< std::wstring >(L"commandArguments", m_commandArguments);
	s >> Member< std::wstring >(L"workingDirectory", m_workingDirectory);
}

	}
}
