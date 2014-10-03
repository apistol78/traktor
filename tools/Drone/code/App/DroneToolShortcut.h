#ifndef traktor_drone_DroneToolShortcut_H
#define traktor_drone_DroneToolShortcut_H

#include "App/DroneTool.h"

namespace traktor
{
	namespace drone
	{

class DroneToolShortcut : public DroneTool
{
	T_RTTI_CLASS

public:
	DroneToolShortcut(
		const std::wstring& title = L"Unnamed",
		const std::wstring& command = L"",
		const std::wstring& commandArguments = L"",
		const std::wstring& workingDirectory = L""
	);

	const std::wstring& getTitle() const;

	const std::wstring& getCommand() const;

	const std::wstring& getCommandArguments() const;

	const std::wstring& getWorkingDirectory() const;

	virtual void getMenuItems(RefArray< ui::MenuItem >& outItems);

	virtual bool execute(ui::Widget* parent, ui::MenuItem* menuItem);

	virtual void serialize(ISerializer& s);

private:
	std::wstring m_title;
	std::wstring m_command;
	std::wstring m_commandArguments;
	std::wstring m_workingDirectory;
};

	}
}

#endif	// traktor_drone_DroneToolShortcut_H
