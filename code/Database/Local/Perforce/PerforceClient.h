/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
#include <clientapi.h>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Io/Path.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Thread/Semaphore.h"
#include "Database/Local/Perforce/PerforceTypes.h"
#include "Database/Local/Perforce/PerforceClientDesc.h"

namespace traktor
{
	namespace db
	{

class PerforceChangeList;
class PerforceChangeListFile;

/*! P4 client.
 * \ingroup Database
 */
class PerforceClient : public Object
{
	T_RTTI_CLASS;

public:
	PerforceClient(const PerforceClientDesc& clientDesc);

	virtual ~PerforceClient();

	bool getChangeLists(RefArray< PerforceChangeList >& outChangeLists);

	Ref< PerforceChangeList > createChangeList(const std::wstring& description);

	bool revertChangeList(PerforceChangeList* changeList);

	bool whereIsLocalFile(const std::wstring& depotFile, std::wstring& outLocalPath);

	bool isOpened(const std::wstring& localFile, PerforceAction& outAction);

	bool addFile(PerforceChangeList* changeList, const std::wstring& localFile);

	bool openForEdit(PerforceChangeList* changeList, const std::wstring& localFile);

	bool openForDelete(PerforceChangeList* changeList, const std::wstring& localFile);

	bool revertFile(PerforceChangeList* changeList, const std::wstring& localFile);

	bool revertUnmodifiedFiles(PerforceChangeList* changeList);

	bool synchronize();

	const std::wstring& getLastError() const;

private:
	PerforceClientDesc m_clientDesc;
	AutoPtr< ClientApi > m_p4client;
	Semaphore m_lock;
	std::wstring m_lastError;

	bool establishConnection();

	bool refreshChangeList(PerforceChangeList* changeList);
};

	}
}

