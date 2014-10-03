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
#include "App/DroneToolP4Backup.h"
#include "App/PerforceClient.h"
#include "App/PerforceChangeList.h"
#include "App/PerforceChangeListFile.h"
#include "App/PerforceChangeListDialog.h"

namespace traktor
{
	namespace drone
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.drone.DroneToolP4Backup", 0, DroneToolP4Backup, DroneTool)

DroneToolP4Backup::DroneToolP4Backup()
:	m_title(L"Backup P4 changelist...")
,	m_verbose(false)
{
}

void DroneToolP4Backup::getMenuItems(RefArray< ui::MenuItem >& outItems)
{
	Ref< ui::MenuItem > menuItem = new ui::MenuItem(ui::Command(L"Drone.Perforce.BackupChangeList"), m_title);
	menuItem->setData(L"TOOL", this);
	outItems.push_back(menuItem);
}

bool DroneToolP4Backup::execute(ui::Widget* parent, ui::MenuItem* menuItem)
{
	if (!m_p4client)
		m_p4client = new PerforceClient(m_clientDesc);

	RefArray< PerforceChangeList > changeLists;
	if (!m_p4client->getChangeLists(changeLists))
		return false;

	PerforceChangeListDialog changeListDialog;
	if (!changeListDialog.create(parent, L"Backup changelist(s)", changeLists))
		return false;
	if (changeListDialog.showModal() != ui::DrOk)
	{
		changeListDialog.destroy();
		return true;
	}
	changeListDialog.getSelectedChangeLists(changeLists);
	changeListDialog.destroy();

	std::wstring backupError = L"";
	bool backupResult = true;

	for (RefArray< PerforceChangeList >::iterator i = changeLists.begin(); i != changeLists.end(); ++i)
	{
		std::wstring backupChangePath = m_backupPath + L"/" + toString((*i)->getChange());
		if (!FileSystem::getInstance().makeDirectory(backupChangePath))
		{
			backupError = L"Unable to create backup directory (1)";
			backupResult = false;
			break;
		}

		for (uint32_t revision = 1; ; ++revision)
		{
			std::wstring backupChangeRevisionPath = backupChangePath + L"/" + toString(revision);
			if (!FileSystem::getInstance().exist(backupChangeRevisionPath))
			{
				backupChangePath = backupChangeRevisionPath;
				break;
			}
		}

		if (!FileSystem::getInstance().makeDirectory(backupChangePath))
		{
			backupError = L"Unable to create backup directory (2)";
			backupResult = false;
			break;
		}

		std::wstring descriptionPath = backupChangePath + L"/ChangeList.xml";

		Ref< IStream > descriptionFile = FileSystem::getInstance().open(descriptionPath, File::FmWrite);
		if (!descriptionFile)
		{
			backupError = L"Unable to create changelist description file";
			backupResult = false;
			break;
		}

		backupResult = xml::XmlSerializer(descriptionFile).writeObject(*i);

		descriptionFile->close();

		if (!backupResult)
		{
			backupError = L"Unable to write changelist description file";
			break;
		}

		const RefArray< PerforceChangeListFile >& changeListFiles = (*i)->getFiles();
		for (RefArray< PerforceChangeListFile >::const_iterator j = changeListFiles.begin(); j != changeListFiles.end(); ++j)
		{
			if ((*j)->getAction() == AtDelete)
				continue;

			std::wstring backupFile = replaceAll((*j)->getDepotPath(), '/', '_');
			std::wstring backupFilePath = backupChangePath+ L"/" + backupFile;

			backupResult = FileSystem::getInstance().copy(
				backupFilePath,
				(*j)->getLocalPath(),
				true
			);
			if (!backupResult)
			{
				backupError = L"Unable to copy file to backup directory";
				break;
			}
		}

		if (!backupResult)
			break;
	}

	if (!backupResult)
		ui::MessageBox::show(parent, backupError, L"Unable to export changelist(s)", ui::MbIconError | ui::MbOk);
	else if (m_verbose)
		ui::MessageBox::show(parent, L"Changelist exported successfully", L"Success", ui::MbIconInformation | ui::MbOk);

	return true;
}

void DroneToolP4Backup::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"title", m_title);
	s >> MemberComposite< PerforceClientDesc >(L"clientDesc", m_clientDesc);
	s >> Member< std::wstring >(L"backupPath", m_backupPath);
	s >> Member< bool >(L"verbose", m_verbose);
}

	}
}
