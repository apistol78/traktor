/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_drone_DroneToolGroup_H
#define traktor_drone_DroneToolGroup_H

#include "Drone/App/DroneTool.h"

namespace traktor
{
	namespace drone
	{

class DroneToolGroup : public DroneTool
{
	T_RTTI_CLASS

public:
	DroneToolGroup(const std::wstring& title = L"Unnamed");

	const std::wstring& getTitle() const;

	virtual void getMenuItems(RefArray< ui::MenuItem >& outItems);

	virtual bool execute(ui::Widget* parent, ui::MenuItem* menuItem);

	virtual void serialize(ISerializer& s);

private:
	std::wstring m_title;
	RefArray< DroneTool > m_tools;
};

	}
}

#endif	// traktor_drone_DroneToolGroup_H
