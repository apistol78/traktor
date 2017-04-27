/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/System/OS.h"
#include "Database/Local/Perforce/PerforceClient.h"
#include "Database/Local/Perforce/PerforceChangeList.h"
#include "Database/Local/Perforce/PerforceChangeListFile.h"
#include "Xml/Document.h"
#include "Xml/Element.h"

namespace traktor
{
	namespace db
	{
		namespace
		{

class ClientUserAdapter : public ClientUser
{
public:
	ClientUserAdapter(std::wstring& outLastError)
	:	m_failed(false)
	,	m_outLastError(outLastError)
	{
	}

	virtual void OutputInfo(char level, const char *data)
	{
		T_DEBUG(L"Perforce info \"" << trim(mbstows(data)) << L"\"");
	}

	virtual void OutputError(const char* errBuf)
	{
		m_failed = true;
		m_outLastError = trim(mbstows(errBuf));
		T_DEBUG(L"Perforce operation failed \"" << m_outLastError << L"\"");
	}

#if defined(_DEBUG)
	virtual void OutputStat(StrDict* varList)
	{
		for (int i = 0; ; ++i)
		{
			StrRef var, val;
			if (!varList->GetVar(i, var, val))
				break;

			T_DEBUG(L"\"" << mbstows(var.Text()) << L"\" => \"" << mbstows(val.Text()) << L"\"");
		}
	}
#endif

	bool failed() const
	{
		return m_failed;
	}

protected:
	bool m_failed;
	std::wstring& m_outLastError;
};

class SynchronizeAdapter : public ClientUserAdapter
{
public:
	SynchronizeAdapter(std::wstring& outLastError)
	:	ClientUserAdapter(outLastError)
	{
	}

	virtual void OutputError(const char* errBuf)
	{
		if (strcmp(errBuf, "File(s) up-to-date.\n") != 0)
			ClientUserAdapter::OutputError(errBuf);
	}
};

class PasswordAdapter : public ClientUserAdapter
{
public:
	PasswordAdapter(std::wstring& outLastError, const std::wstring& password)
	:	ClientUserAdapter(outLastError)
	{
		strcpy_s(m_password, wstombs(password).c_str());
	}

	virtual void InputData(StrBuf* strBuf, Error* e)
	{
		strBuf->Set(m_password);
	}

private:
	char m_password[64];
};

class ChangeListAdapter : public ClientUserAdapter
{
public:
	ChangeListAdapter(std::wstring& outLastError, RefArray< PerforceChangeList >& outChangeLists)
	:	ClientUserAdapter(outLastError)
	,	m_outChangeLists(outChangeLists)
	{
	}

	virtual void OutputStat(StrDict* varList)
	{
		ClientUserAdapter::OutputStat(varList);

		StrPtr* change = varList->GetVar("change");
		StrPtr* user = varList->GetVar("user");
		StrPtr* client = varList->GetVar("client");
		StrPtr* description = varList->GetVar("desc");
		if (change && user && client && description)
		{
			m_outChangeLists.push_back(new PerforceChangeList(
				change->Atoi(),
				trim(mbstows(user->Text())),
				trim(mbstows(client->Text())),
				trim(mbstows(description->Text()))
			));
		}
	}

private:
	RefArray< PerforceChangeList >& m_outChangeLists;
};

class ChangeListDepotFilesAdapter : public ClientUserAdapter
{
public:
	ChangeListDepotFilesAdapter(std::wstring& outLastError, RefArray< PerforceChangeListFile >& changeListFiles)
	:	ClientUserAdapter(outLastError)
	,	m_outChangeListFiles(changeListFiles)
	{
	}

	virtual void OutputStat(StrDict* varList)
	{
		ClientUserAdapter::OutputStat(varList);

		for (int i = 0; ; ++i)
		{
			char buf[256];

			sprintf_s(buf, "action%d", i);
			StrPtr* action = varList->GetVar(buf);

			sprintf_s(buf, "depotFile%d", i);
			StrPtr* depotFile = varList->GetVar(buf);

			if (!action || !depotFile)
				break;

			Ref< PerforceChangeListFile > changeListFile = new PerforceChangeListFile();

			changeListFile->setDepotPath(mbstows(depotFile->Text()));

			if (*action == "add")
				changeListFile->setAction(AtAdd);
			else if (*action == "edit")
				changeListFile->setAction(AtEdit);
			else if (*action == "delete")
				changeListFile->setAction(AtDelete);

			m_outChangeListFiles.push_back(changeListFile);
		}
	}

private:
	RefArray< PerforceChangeListFile >& m_outChangeListFiles;
};

class DepotToWorkspaceFileAdapter : public ClientUserAdapter
{
public:
	DepotToWorkspaceFileAdapter(std::wstring& outLastError, std::wstring& outLocalPath)
	:	ClientUserAdapter(outLastError)
	,	m_outLocalPath(outLocalPath)
	{
	}

	virtual void OutputStat(StrDict* varList)
	{
		ClientUserAdapter::OutputStat(varList);

		StrPtr* path = varList->GetVar("path");
		if (path)
			m_outLocalPath = mbstows(path->Text());
	}

private:
	std::wstring& m_outLocalPath;
};

class FileStatusAdapter : public ClientUserAdapter
{
public:
	FileStatusAdapter(std::wstring& outLastError, PerforceAction& outAction)
	:	ClientUserAdapter(outLastError)
	,	m_outAction(outAction)
	{
		m_outAction = AtNotOpened;
	}

	virtual void OutputError(const char* errBuf)
	{
		m_outAction = AtNotOpened;
		ClientUserAdapter::OutputError(errBuf);
	}

	virtual void OutputStat(StrDict* varList)
	{
		ClientUserAdapter::OutputStat(varList);

		StrPtr* action = varList->GetVar("action");
		if (!action)
			return;

		if (*action == "add")
			m_outAction = AtAdd;
		else if (*action == "edit")
			m_outAction = AtEdit;
		else if (*action == "delete")
			m_outAction = AtDelete;
		else
			m_outAction = AtNotOpened;
	}

private:
	PerforceAction& m_outAction;
};

class CreateChangeListAdapter : public ClientUserAdapter
{
public:
	CreateChangeListAdapter(std::wstring& outLastError, const std::wstring& client, const std::wstring& user, const std::wstring& description)
	:	ClientUserAdapter(outLastError)
	,	m_change(0)
	{
		StringOutputStream ss;

		ss << L"# A Perforce Change Specification." << Endl;
		ss << L"#" << Endl;
		ss << L"#  Change:      The change number. 'new' on a new changelist." << Endl;
		ss << L"#  Date:        The date this specification was last modified." << Endl;
		ss << L"#  Client:      The client on which the changelist was created.  Read-only." << Endl;
		ss << L"#  User:        The user who created the changelist." << Endl;
		ss << L"#  Status:      Either 'pending' or 'submitted'. Read-only." << Endl;
		ss << L"#  Description: Comments about the changelist.  Required." << Endl;
		ss << L"#  Jobs:        What opened jobs are to be closed by this changelist." << Endl;
		ss << L"#               You may delete jobs from this list.  (New changelists only.)" << Endl;
		ss << L"#  Files:       What opened files from the default changelist are to be added" << Endl;
		ss << L"#               to this changelist.  You may delete files from this list." << Endl;
		ss << L"#               (New changelists only.)" << Endl;
		ss << L"" << Endl;
		ss << L"Change:	new" << Endl;
		ss << L"" << Endl;
		ss << L"Client:	" << client << Endl;
		ss << L"" << Endl;
		ss << L"User:	" << user << Endl;
		ss << L"" << Endl;
		ss << L"Status:	new" << Endl;
		ss << L"" << Endl;
		ss << L"Description:" << Endl;
		ss << L"	" << description << Endl;

		strcpy_s(m_description, wstombs(ss.str()).c_str());
	}

	virtual void OutputInfo(char level, const char *data)
	{
		std::wstring info = mbstows(data);
		if (startsWith< std::wstring >(info, L"Change "))
			m_change = parseString< uint32_t >(info.substr(7));
		else
			ClientUserAdapter::OutputInfo(level, data);
	}

	virtual void InputData(StrBuf* strBuf, Error* e)
	{
		strBuf->Set(m_description);
	}

	uint32_t getChange() const
	{
		return m_change;
	}

private:
	char m_description[4096];
	uint32_t m_change;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.PerforceClient", PerforceClient, Object)

PerforceClient::PerforceClient(const PerforceClientDesc& clientDesc)
:	m_clientDesc(clientDesc)
,	m_lastError(L"No error")
{
}

PerforceClient::~PerforceClient()
{
	if (m_p4client.ptr())
	{
		Error e;
		m_p4client->Final(&e);
		m_p4client.release();
	}
}

bool PerforceClient::getChangeLists(RefArray< PerforceChangeList >& outChangeLists)
{
	if (!establishConnection())
		return false;

	outChangeLists.resize(0);

	{
		ChangeListAdapter changeListAdapter(m_lastError, outChangeLists);

		char client[256];
		strcpy_s(client, wstombs(m_clientDesc.m_client).c_str());

		char* const argv[] = { "-s", "pending", "-c", client };

		m_p4client->SetArgv(sizeof_array(argv), argv);
		m_p4client->Run("changes", &changeListAdapter);

		if (changeListAdapter.failed())
			return false;
	}

	for (RefArray< PerforceChangeList >::iterator i = outChangeLists.begin(); i != outChangeLists.end(); ++i)
	{
		RefArray< PerforceChangeListFile > changeListFiles;

		{
			ChangeListDepotFilesAdapter depotFilesAdapter(m_lastError, changeListFiles);

			char change[256];
			sprintf_s(change, "%d", (*i)->getChange());

			char* const argv[] = { "-s", change };

			m_p4client->SetArgv(sizeof_array(argv), argv);
			m_p4client->Run("describe", &depotFilesAdapter);

			if (depotFilesAdapter.failed())
				return false;
		}

		for (RefArray< PerforceChangeListFile >::iterator j = changeListFiles.begin(); j != changeListFiles.end(); ++j)
		{
			std::wstring localPath;

			{
				DepotToWorkspaceFileAdapter changeListFileAdapter(m_lastError, localPath);

				char depotFile[256];
				strcpy_s(depotFile, wstombs((*j)->getDepotPath()).c_str());

				char* const argv[] = { depotFile };
				m_p4client->SetArgv(sizeof_array(argv), argv);
				m_p4client->Run("where", &changeListFileAdapter);

				if (changeListFileAdapter.failed())
					return false;
			}

			(*j)->setLocalPath(localPath);
		}

		(*i)->setFiles(changeListFiles);
	}

	return true;
}

Ref< PerforceChangeList > PerforceClient::createChangeList(const std::wstring& description)
{
	if (!establishConnection())
		return 0;

	CreateChangeListAdapter createChangeListAdapter(m_lastError, m_clientDesc.m_client, m_clientDesc.m_user, description);

	char* const argv[] = { "-i" };
	m_p4client->SetArgv(sizeof_array(argv), argv);
	m_p4client->Run("change", &createChangeListAdapter);

	if (createChangeListAdapter.failed())
		return 0;

	return new PerforceChangeList(
		createChangeListAdapter.getChange(),
		m_clientDesc.m_user,
		m_clientDesc.m_client,
		description
	);
}

bool PerforceClient::whereIsLocalFile(const std::wstring& depotFile, std::wstring& outLocalPath)
{
	if (!establishConnection())
		return false;

	char file[256];
	strcpy_s(file, wstombs(depotFile).c_str());

	DepotToWorkspaceFileAdapter changeListFileAdapter(m_lastError, outLocalPath);

	{
		char* const argv[] = { file };
		m_p4client->SetArgv(sizeof_array(argv), argv);
		m_p4client->Run("where", &changeListFileAdapter);
	}

	if (changeListFileAdapter.failed())
		return false;

	return true;
}

bool PerforceClient::isOpened(const std::wstring& localFile, PerforceAction& outAction)
{
	if (!establishConnection())
		return false;

	char file[256];
	strcpy_s(file, wstombs(localFile).c_str());

	FileStatusAdapter fileStatusAdapter(m_lastError, outAction);

	{
		char* const argv[] = { file };
		m_p4client->SetArgv(sizeof_array(argv), argv);
		m_p4client->Run("fstat", &fileStatusAdapter);
	}

	if (fileStatusAdapter.failed())
		return false;

	return true;
}

bool PerforceClient::addFile(const PerforceChangeList* changeList, const std::wstring& localFile)
{
	if (!establishConnection())
		return false;

	ClientUserAdapter userAdapter(m_lastError);

	char change[256];
	sprintf_s(change, "%d", changeList->getChange());

	char file[256];
	strcpy_s(file, wstombs(localFile).c_str());

	{
		char* const argv[] = { "-c", change, file };
		m_p4client->SetArgv(sizeof_array(argv), argv);
		m_p4client->Run("add", &userAdapter);
	}

	if (userAdapter.failed())
		return false;

	return true;
}

bool PerforceClient::openForEdit(const PerforceChangeList* changeList, const std::wstring& localFile)
{
	if (!establishConnection())
		return false;

	ClientUserAdapter userAdapter(m_lastError);

	char change[256];
	sprintf_s(change, "%d", changeList->getChange());

	char file[256];
	strcpy_s(file, wstombs(localFile).c_str());

	{
		char* const argv[] = { "-c", change, file };
		m_p4client->SetArgv(sizeof_array(argv), argv);
		m_p4client->Run("edit", &userAdapter);
	}

	if (userAdapter.failed())
		return false;

	return true;
}

bool PerforceClient::openForDelete(const PerforceChangeList* changeList, const std::wstring& localFile)
{
	if (!establishConnection())
		return false;

	ClientUserAdapter userAdapter(m_lastError);

	char change[256];
	sprintf_s(change, "%d", changeList->getChange());

	char file[256];
	strcpy_s(file, wstombs(localFile).c_str());

	{
		char* const argv[] = { "-c", change, file };
		m_p4client->SetArgv(sizeof_array(argv), argv);
		m_p4client->Run("delete", &userAdapter);
	}

	if (userAdapter.failed())
		return false;

	return true;
}

bool PerforceClient::revertFile(const PerforceChangeList* changeList, const std::wstring& localFile)
{
	if (!establishConnection())
		return false;

	ClientUserAdapter userAdapter(m_lastError);

	char change[256];
	sprintf_s(change, "%d", changeList->getChange());

	char file[256];
	strcpy_s(file, wstombs(localFile).c_str());

	{
		char* const argv[] = { "-c", change, file };
		m_p4client->SetArgv(sizeof_array(argv), argv);
		m_p4client->Run("revert", &userAdapter);
	}

	if (userAdapter.failed())
		return false;

	return true;
}

bool PerforceClient::synchronize()
{
	if (!establishConnection())
		return false;

	SynchronizeAdapter synchronizeAdapter(m_lastError);

	{
		m_p4client->Clear();
		m_p4client->Run("sync", &synchronizeAdapter);
	}

	if (synchronizeAdapter.failed())
		return false;

	return true;
}

const std::wstring& PerforceClient::getLastError() const
{
	return m_lastError;
}

bool PerforceClient::establishConnection()
{
	Error e;

	if (m_p4client.ptr() != 0 && !m_p4client->Dropped())
		return true;

	m_p4client.reset(new ClientApi());
	m_p4client->SetProtocol("tag", "");
	m_p4client->SetProg("Traktor.Editor");
	m_p4client->SetVersion("1.0");

	if (!m_clientDesc.m_host.empty())
		m_p4client->SetHost(wstombs(m_clientDesc.m_host).c_str());
	if (!m_clientDesc.m_port.empty())
		m_p4client->SetPort(wstombs(m_clientDesc.m_port).c_str());
	if (!m_clientDesc.m_user.empty())
		m_p4client->SetUser(wstombs(m_clientDesc.m_user).c_str());
	if (!m_clientDesc.m_client.empty())
		m_p4client->SetClient(wstombs(m_clientDesc.m_client).c_str());
	else
	{
		// No client specified; read last used client from p4v application settings.
		Path applicationSettingsPath = OS::getInstance().getUserHomePath() + L"/../.p4qt/ApplicationSettings.xml";
		Ref< xml::Document > doc = new xml::Document();
		if (doc->loadFromFile(applicationSettingsPath))
		{
			Ref< xml::Element > lastConnection = doc->getSingle(L"/PropertyList/PropertyList[@varName=Connection]/String[@varName=LastConnection]");
			if (lastConnection)
			{
				std::wstring v = lastConnection->getValue();
				size_t p = v.find_last_of(L',');
				if (p != v.npos)
					m_clientDesc.m_client = v.substr(p + 2);
			}
		}

		if (m_clientDesc.m_client.empty())
			return false;

		m_p4client->SetClient(wstombs(m_clientDesc.m_client).c_str());
	}

	log::info << L"Perforce: Initialize client..." << Endl;
	if (!m_clientDesc.m_host.empty())
		log::info << L"\tHost \"" << m_clientDesc.m_host << L"\"" << Endl;
	if (!m_clientDesc.m_port.empty())
		log::info << L"\tPort \"" << m_clientDesc.m_port << L"\"" << Endl;
	if (!m_clientDesc.m_user.empty())
		log::info << L"\tUser \"" << m_clientDesc.m_user << L"\"" << Endl;
	if (!m_clientDesc.m_client.empty())
		log::info << L"\tClient \"" << m_clientDesc.m_client << L"\"" << Endl;

	m_p4client->Init(&e);
	if (e.Test())
	{
		StrBuf buffer;
		e.Fmt(&buffer);

		log::error << L"Perforce: Unable to connect, \"" << mbstows(buffer.Text()) << L"\"" << Endl;
		m_lastError = mbstows(buffer.Text());

		m_p4client.release();
		return false;
	}

	if (!m_clientDesc.m_password.empty())
	{
		log::info << L"Perforce: Login client..." << Endl;

		PasswordAdapter passwordAdapter(m_lastError, m_clientDesc.m_password);

		char user[256];
		strcpy_s(user, wstombs(m_clientDesc.m_user).c_str());

		char* const argv[] = { user };
		m_p4client->SetArgv(sizeof_array(argv), argv);
		if (m_clientDesc.m_securityLevel == PerforceClientDesc::SlLow)
			m_p4client->SetPassword(wstombs(m_clientDesc.m_password).c_str());
		m_p4client->Run("login", &passwordAdapter);

		if (passwordAdapter.failed())
		{
			m_p4client.release();
			return false;
		}
	}

	m_clientDesc.m_host = mbstows(m_p4client->GetHost().Text());
	m_clientDesc.m_port = mbstows(m_p4client->GetPort().Text());
	m_clientDesc.m_user = mbstows(m_p4client->GetUser().Text());

	log::info << L"Perforce: Connected successfully" << Endl;
	return true;
}

	}
}
