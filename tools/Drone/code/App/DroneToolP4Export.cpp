#include <Ui/MenuItem.h>
#include <Ui/MessageBox.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/IStream.h>
#include <Core/Io/StringOutputStream.h>
#include <Core/Serialization/MemberComposite.h>
#include <Core/Misc/String.h>
#include <Core/Log/Log.h>
#include <Xml/XmlSerializer.h>
#include <Xml/XmlDeserializer.h>
#include "App/DroneToolP4Export.h"
#include "App/PerforceClient.h"
#include "App/PerforceChangeList.h"
#include "App/PerforceChangeListFile.h"
#include "App/PerforceChangeListDialog.h"

namespace traktor
{
	namespace drone
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.drone.DroneToolP4Export", 0, DroneToolP4Export, DroneTool)

DroneToolP4Export::DroneToolP4Export()
:	m_title(L"Export P4 changelist...")
,	m_verbose(false)
{
}

void DroneToolP4Export::getMenuItems(RefArray< ui::MenuItem >& outItems)
{
	Ref< ui::MenuItem > menuItem = new ui::MenuItem(ui::Command(L"Drone.Perforce.ExportChangeList"), m_title);
	menuItem->setData(L"TOOL", this);
	outItems.push_back(menuItem);
}

bool DroneToolP4Export::execute(ui::Widget* parent, ui::MenuItem* menuItem)
{
	if (!m_p4client)
		m_p4client = new PerforceClient(m_clientDesc);

	RefArray< PerforceChangeList > changeLists;
	if (!m_p4client->getChangeLists(changeLists))
		return false;

	PerforceChangeListDialog changeListDialog;
	if (!changeListDialog.create(parent, L"Export changelist(s)", changeLists))
		return false;
	if (changeListDialog.showModal() != ui::DrOk)
	{
		changeListDialog.destroy();
		return true;
	}
	changeListDialog.getSelectedChangeLists(changeLists);
	changeListDialog.destroy();

	std::wstring exportError = L"";
	bool exportResult = true;

	for (RefArray< PerforceChangeList >::iterator i = changeLists.begin(); i != changeLists.end(); ++i)
	{
		Path exportPath = m_exportPath + L"/" + toString((*i)->getChange());
		if (!FileSystem::getInstance().makeDirectory(exportPath))
		{
			exportError = L"Unable to create export directory";
			exportResult = false;
			break;
		}

		std::wstring descriptionFilePath = exportPath.getPathName() + L"/ChangeList.xml";
		Ref< IStream > descriptionFile = FileSystem::getInstance().open(descriptionFilePath, File::FmWrite);
		if (!descriptionFile)
		{
			exportError = L"Unable to create changelist description file";
			exportResult = false;
			break;
		}

		exportResult = xml::XmlSerializer(descriptionFile).writeObject(*i);

		descriptionFile->close();

		if (!exportResult)
		{
			exportError = L"Unable to write changelist description file";
			break;
		}

		const RefArray< PerforceChangeListFile >& changeListFiles = (*i)->getFiles();
		for (RefArray< PerforceChangeListFile >::const_iterator j = changeListFiles.begin(); j != changeListFiles.end(); ++j)
		{
			if ((*j)->getAction() == AtDelete)
				continue;

			std::wstring exportFile = replaceAll((*j)->getDepotPath(), '/', '_');
			std::wstring exportFilePath = exportPath.getPathName() + L"/" + exportFile;

			exportResult = FileSystem::getInstance().copy(
				exportFilePath,
				(*j)->getLocalPath(),
				true
			);
			if (!exportResult)
			{
				exportError = L"Unable to copy file to export directory";
				break;
			}
		}

		if (!exportResult)
			break;
	}

	if (!exportResult)
		ui::MessageBox::show(parent, exportError, L"Unable to export changelist(s)", ui::MbIconError | ui::MbOk);
	else if (m_verbose)
		ui::MessageBox::show(parent, L"Changelist exported successfully", L"Success", ui::MbIconInformation | ui::MbOk);

	return true;
}

void DroneToolP4Export::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"title", m_title);
	s >> MemberComposite< PerforceClientDesc >(L"clientDesc", m_clientDesc);
	s >> Member< std::wstring >(L"exportPath", m_exportPath);
	s >> Member< bool >(L"verbose", m_verbose);
}

	}
}
