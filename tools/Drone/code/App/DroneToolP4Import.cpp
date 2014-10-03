#include <Ui/MenuItem.h>
#include <Ui/MessageBox.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/IStream.h>
#include <Core/Io/StringOutputStream.h>
#include <Core/System/OS.h>
#include <Core/System/IProcess.h>
#include <Core/Serialization/MemberComposite.h>
#include <Core/Misc/String.h>
#include <Core/Misc/TString.h>
#include <Core/Log/Log.h>
#include <Xml/XmlSerializer.h>
#include <Xml/XmlDeserializer.h>
#include "App/DroneToolP4Import.h"
#include "App/PerforceClient.h"
#include "App/PerforceChangeList.h"
#include "App/PerforceChangeListFile.h"
#include "App/PerforceChangeListDialog.h"

namespace traktor
{
	namespace drone
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.drone.DroneToolP4Import", 0, DroneToolP4Import, DroneTool)

DroneToolP4Import::DroneToolP4Import()
:	m_title(L"Import P4 changelist...")
,	m_mergeExecutable(L"p4merge.exe")
,	m_verbose(false)
{
}

void DroneToolP4Import::getMenuItems(RefArray< ui::MenuItem >& outItems)
{
	Ref< ui::MenuItem > menuItem = new ui::MenuItem(ui::Command(L"Drone.Perforce.ImportChangeList"), m_title);
	menuItem->setData(L"TOOL", this);
	outItems.push_back(menuItem);
}

bool DroneToolP4Import::execute(ui::Widget* parent, ui::MenuItem* menuItem)
{
	if (!m_p4client)
		m_p4client = new PerforceClient(m_clientDesc);

	RefArray< File > files;
	FileSystem::getInstance().find(std::wstring(m_exportPath) + L"/*.*", files);

	RefArray< PerforceChangeList > changeLists;
	for (RefArray< File >::iterator i = files.begin(); i != files.end(); ++i)
	{
		if (!(*i)->isDirectory())
			continue;

		std::wstring descriptionFilePath = (*i)->getPath().getPathName() + L"/ChangeList.xml";
		Ref< IStream > descriptionFile = FileSystem::getInstance().open(descriptionFilePath, File::FmRead);
		if (!descriptionFile)
			continue;

		Ref< PerforceChangeList > changeList = xml::XmlDeserializer(descriptionFile).readObject< PerforceChangeList >();

		descriptionFile->close();

		if (changeList)
			changeLists.push_back(changeList);
	}

	if (changeLists.empty())
		return true;

	PerforceChangeListDialog changeListDialog;
	if (!changeListDialog.create(parent, L"Import changelist(s)", changeLists))
		return false;
	if (changeListDialog.showModal() != ui::DrOk)
	{
		changeListDialog.destroy();
		return true;
	}
	changeListDialog.getSelectedChangeLists(changeLists);
	changeListDialog.destroy();

	std::wstring importError = L"";
	bool importResult = true;

	for (RefArray< PerforceChangeList >::iterator i = changeLists.begin(); i != changeLists.end(); ++i)
	{
		Ref< PerforceChangeList > changeList = m_p4client->createChangeList((*i)->getDescription());
		if (!changeList)
		{
			importError = L"Unable to create changelist.";
			importResult = false;
			break;
		}

		Path exportPath = std::wstring(m_exportPath) + L"/" + toString((*i)->getChange());

		const RefArray< PerforceChangeListFile >& changeListFiles = (*i)->getFiles();
		for (RefArray< PerforceChangeListFile >::const_iterator j = changeListFiles.begin(); j != changeListFiles.end(); ++j)
		{
			PerforceAction importAction = (*j)->getAction();
			std::wstring depotFile = (*j)->getDepotPath();
			std::wstring importFile = m_exportPath + L"/" + toString((*i)->getChange()) + L"/" + replaceAll(depotFile, '/', '_');
			std::wstring localFile;
			PerforceAction localAction;

			if (!m_p4client->whereIsLocalFile(depotFile, localFile))
			{
				importError = L"Unable to locate file in client view.";
				importResult = false;
				break;
			}

			if (!m_p4client->isOpened(localFile, localAction))
			{
				importError = L"Unable to query file status.";
				importResult = false;
				break;
			}

			if (importAction == AtAdd)
			{
				if (localAction != AtNotOpened)
				{
					importError = L"Cannot add file \"" + localFile + L"\" as it's already opened in another changelist.";
					importResult = false;
					break;
				}
			}
			else if (importAction == AtEdit)
			{
				if (localAction != AtNotOpened)
				{
					if (localAction == AtDelete)
					{
						importError = L"Cannot add file \"" + localFile + L"\" as it's already opened for delete in another changelist.";
						importResult = false;
						break;
					}

					char mergeFile[256];
					tmpnam_s(mergeFile);

					if (!FileSystem::getInstance().copy(
						mbstows(mergeFile),
						localFile,
						true
					))
					{
						importError = L"Unable to prepare merge operation.";
						importResult = false;
						break;
					}

					StringOutputStream ss;
					ss << m_mergeExecutable << L" ";
					ss << Path(localFile).getPathName() << L" ";	// Base
					ss << Path(importFile).getPathName() << L" ";	// Left
					ss << Path(localFile).getPathName() << L" ";	// Right
					ss << mbstows(mergeFile);		// Merge

					Ref< IProcess > process = OS::getInstance().execute(
						ss.str(),
						L"",
						0,
						false,
						false,
						false
					);
					if (!process)
					{
						importError = L"File \"" + localFile + L"\" already opened in another changelist, unable to start merge tool.";
						importResult = false;
						break;
					}
					process->wait();

					importFile = mbstows(mergeFile);
				}
				else
				{
					if (!m_p4client->openForEdit(changeList, localFile))
					{
						importError = L"Unable to open file \"" + localFile + L"\" for edit.";
						importResult = false;
						break;
					}
				}
			}
			else if (importAction == AtDelete)
			{
				if (localAction != AtNotOpened && localAction != AtDelete)
				{
					importError = L"Cannot delete file \"" + localFile + L"\" as it's already opened in another changelist.";
					importResult = false;
					break;
				}

				if (!m_p4client->openForDelete(changeList, localFile))
				{
					importError = L"Unable to open file \"" + localFile + L"\" for delete.";
					importResult = false;
					break;
				}
			}

			if (importAction == AtAdd || importAction == AtEdit)
			{
				if (!FileSystem::getInstance().copy(
					localFile,
					importFile,
					true
				))
				{
					importError = L"Unable to copy file \"" + importFile + L"\" to local view.";
					importResult = false;
					break;
				}

				if (importAction == AtAdd)
				{
					if (!m_p4client->addFile(changeList, localFile))
					{
						importError = L"Unable to add file \"" + std::wstring(localFile) + L"\" to changelist.";
						importResult = false;
						break;
					}
				}
			}
		}

		if (!importResult)
			break;
	}

	if (!importResult)
		ui::MessageBox::show(parent, importError + L"\nImported changelist may be incomplete.", L"Unable to import changelist(s)", ui::MbIconError | ui::MbOk);
	else if (m_verbose)
		ui::MessageBox::show(parent, L"Changelist imported successfully", L"Success", ui::MbIconInformation | ui::MbOk);

	return true;
}

void DroneToolP4Import::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"title", m_title);
	s >> MemberComposite< PerforceClientDesc >(L"clientDesc", m_clientDesc);
	s >> Member< std::wstring >(L"exportPath", m_exportPath);
	s >> Member< std::wstring >(L"mergeExecutable", m_mergeExecutable);
	s >> Member< bool >(L"verbose", m_verbose);
}

	}
}
