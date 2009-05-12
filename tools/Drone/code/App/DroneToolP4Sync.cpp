#include <Ui/MenuItem.h>
#include <Ui/MessageBox.h>
#include <Core/Serialization/Serializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberAggregate.h>
#include "App/DroneToolP4Sync.h"
#include "App/PerforceClient.h"

namespace traktor
{
	namespace drone
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.drone.DroneToolP4Sync", DroneToolP4Sync, DroneTool)

DroneToolP4Sync::DroneToolP4Sync()
:	m_title(L"Sync P4...")
,	m_verbose(false)
{
}

void DroneToolP4Sync::getMenuItems(RefArray< ui::MenuItem >& outItems)
{
	Ref< ui::MenuItem > menuItem = gc_new< ui::MenuItem >(ui::Command(L"Drone.Perforce.Sync"), m_title);
	menuItem->setData(L"TOOL", this);
	outItems.push_back(menuItem);
}

bool DroneToolP4Sync::execute(ui::Widget* parent, ui::MenuItem* menuItem)
{
	if (!m_p4client)
		m_p4client = gc_new< PerforceClient >(cref(m_clientDesc));

	if (!m_p4client->synchronize())
		ui::MessageBox::show(
			parent,
			L"Failed to synchronize client view;\n" + m_p4client->getLastError(),
			L"Unable to synchronize",
			ui::MbIconError | ui::MbOk
		);
	else if (m_verbose)
		ui::MessageBox::show(parent, L"Synchronization successful", L"Success", ui::MbIconInformation | ui::MbOk);

	return true;
}

bool DroneToolP4Sync::serialize(Serializer& s)
{
	s >> Member< std::wstring >(L"title", m_title);
	s >> MemberAggregate< PerforceClientDesc >(L"clientDesc", m_clientDesc);
	s >> Member< bool >(L"verbose", m_verbose);
	return true;
}

	}
}
