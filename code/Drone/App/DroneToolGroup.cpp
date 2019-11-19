#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Drone/App/DroneToolGroup.h"
#include "Ui/MenuItem.h"

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

	for (auto tool : m_tools)
	{
		RefArray< ui::MenuItem > menuItems;
		tool->getMenuItems(menuItems);
		for (auto menuItem : menuitems)
			menuItem->add(menuItem);
	}

	outItems.push_back(menuItem);
}

bool DroneToolGroup::execute(ui::Widget* parent, const ui::MenuItem* menuItem)
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
