/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberRefArray.h>
#include <Ui/MenuItem.h>
#include "Drone/App/DroneToolGroup.h"
#include "Drone/App/Process.h"

namespace traktor
{
	namespace drone
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.drone.DroneToolGroup", 0, DroneToolGroup, DroneTool)

DroneToolGroup::DroneToolGroup(const std::wstring& title)
:	m_title(title)
{
}

const std::wstring& DroneToolGroup::getTitle() const
{
	return m_title;
}

void DroneToolGroup::getMenuItems(RefArray< ui::MenuItem >& outItems)
{
	Ref< ui::MenuItem > menuItem = new ui::MenuItem(m_title);
	menuItem->setData(L"TOOL", this);

	for (RefArray< DroneTool >::iterator i = m_tools.begin(); i != m_tools.end(); ++i)
	{
		RefArray< ui::MenuItem > menuItems;
		(*i)->getMenuItems(menuItems);

		for (RefArray< ui::MenuItem >::iterator j = menuItems.begin(); j != menuItems.end(); ++j)
			menuItem->add(*j);
	}

	outItems.push_back(menuItem);
}

bool DroneToolGroup::execute(ui::Widget* parent, ui::MenuItem* menuItem)
{
	return false;
}

void DroneToolGroup::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"title", m_title);
	s >> MemberRefArray< DroneTool >(L"tools", m_tools);
}

	}
}
