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
#include "App/DroneToolP4Restore.h"
#include "App/PerforceClient.h"
#include "App/PerforceChangeList.h"
#include "App/PerforceChangeListFile.h"
#include "App/PerforceChangeListDialog.h"

namespace traktor
{
	namespace drone
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.drone.DroneToolP4Restore", 0, DroneToolP4Restore, DroneTool)

DroneToolP4Restore::DroneToolP4Restore()
:	m_title(L"Restore P4 changelist...")
,	m_verbose(false)
{
}

void DroneToolP4Restore::getMenuItems(RefArray< ui::MenuItem >& outItems)
{
	Ref< ui::MenuItem > menuItem = new ui::MenuItem(ui::Command(L"Drone.Perforce.RestoreChangeList"), m_title);
	menuItem->setData(L"TOOL", this);
	outItems.push_back(menuItem);
}

bool DroneToolP4Restore::execute(ui::Widget* parent, ui::MenuItem* menuItem)
{
	if (!m_p4client)
		m_p4client = new PerforceClient(m_clientDesc);

	RefArray< File > changeListDirectories;
	FileSystem::getInstance().find(m_backupPath + L"/*.*", changeListDirectories);

//	RefArray< PerforceChangeList > changeLists;
	for (RefArray< File >::iterator i = changeListDirectories.begin(); i != changeListDirectories.end(); ++i)
	{
		if (!(*i)->isDirectory())
			continue;

		//std::wstring descriptionFilePath = std::wstring((*i)->getPath()) + L"/ChangeList.xml";
		//Ref< Stream > descriptionFile = FileSystem::getInstance().open(descriptionFilePath, File::FmRead);
		//if (!descriptionFile)
		//	continue;

		//Ref< PerforceChangeList > changeList = xml::XmlDeserializer(descriptionFile).readObject< PerforceChangeList >();

		//descriptionFile->close();

		//if (changeList)
		//	changeLists.push_back(changeList);
	}

	//if (changeLists.empty())
	//	return true;

	//PerforceChangeListDialog changeListDialog;
	//if (!changeListDialog.create(L"Import changelist(s)", changeLists))
	//	return false;
	//if (changeListDialog.showModal() != ui::DrOk)
	//{
	//	changeListDialog.destroy();
	//	return true;
	//}
	//changeListDialog.getSelectedChangeLists(changeLists);
	//changeListDialog.destroy();

	//std::wstring importError = L"";
	//bool importResult = true;

	//for (RefArray< PerforceChangeList >::iterator i = changeLists.begin(); i != changeLists.end(); ++i)
	//{
	//	Ref< PerforceChangeList > changeList = m_p4client->createChangeList((*i)->getDescription());
	//	if (!changeList)
	//	{
	//		importError = L"Unable to create changelist.";
	//		importResult = false;
	//		break;
	//	}

	//	Path exportPath = std::wstring(m_exportPath) + L"/" + toString((*i)->getChange());

	//	const RefArray< PerforceChangeListFile >& changeListFiles = (*i)->getFiles();
	//	for (RefArray< PerforceChangeListFile >::const_iterator j = changeListFiles.begin(); j != changeListFiles.end(); ++j)
	//	{
	//		PerforceChangeListFile::Action action = (*j)->getAction();
	//		std::wstring depotFile = (*j)->getDepotPath();
	//		std::wstring localFile;

	//		if (!m_p4client->whereIsLocalFile(depotFile, localFile))
	//		{
	//			importError = L"Unable to locate file in client view.";
	//			importResult = false;
	//			break;
	//		}

	//		if (action == PerforceChangeListFile::AtEdit || action == PerforceChangeListFile::AtDelete)
	//		{
	//			if (action == PerforceChangeListFile::AtEdit)
	//			{
	//				if (!m_p4client->openForEdit(changeList, localFile))
	//				{
	//					importError = L"Unable to open file \"" + localFile + L"\" for edit.";
	//					importResult = false;
	//					break;
	//				}
	//			}
	//			else	// AtDelete
	//			{
	//				if (!m_p4client->openForDelete(changeList, localFile))
	//				{
	//					importError = L"Unable to open file \"" + localFile + L"\" for delete.";
	//					importResult = false;
	//					break;
	//				}
	//			}
	//		}

	//		if (action == PerforceChangeListFile::AtAdd || action == PerforceChangeListFile::AtEdit)
	//		{
	//			std::wstring sharedFile = replaceAll(depotFile, '/', '_');

	//			StringOutputStream ss;
	//			ss << m_exportPath << L"/" << (*i)->getChange() << L"/" << sharedFile;

	//			if (!FileSystem::getInstance().copy(
	//				localFile,
	//				ss.str(),
	//				true
	//				))
	//			{
	//				importError = L"Unable to copy file \"" + sharedFile + L"\" to local view.";
	//				importResult = false;
	//				break;
	//			}

	//			if (action == PerforceChangeListFile::AtAdd)
	//			{
	//				if (!m_p4client->addFile(changeList, localFile))
	//				{
	//					importError = L"Unable to add file \"" + std::wstring(localFile) + L"\" to changelist.";
	//					importResult = false;
	//					break;
	//				}
	//			}
	//		}
	//	}

	//	if (!importResult)
	//		break;
	//}

	//if (!importResult)
	//	ui::MessageBox::show(parent, importError + L"\nImport changelist may be incomplete.", L"Unable to import changelist(s)", ui::MbIconError | ui::MbOk);
	//else if (m_verbose)
	//	ui::MessageBox::show(parent, L"Changelist imported successfully", L"Success", ui::MbIconInformation | ui::MbOk);

	return true;
}

void DroneToolP4Restore::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"title", m_title);
	s >> MemberComposite< PerforceClientDesc >(L"clientDesc", m_clientDesc);
	s >> Member< std::wstring >(L"backupPath", m_backupPath);
	s >> Member< bool >(L"verbose", m_verbose);
}

	}
}
