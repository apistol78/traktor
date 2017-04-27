/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_PerforceClient_H
#define traktor_db_PerforceClient_H

#include <vector>
#include <clientapi.h>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Io/Path.h"
#include "Core/Misc/AutoPtr.h"
#include "Database/Local/Perforce/PerforceTypes.h"
#include "Database/Local/Perforce/PerforceClientDesc.h"

namespace traktor
{
	namespace db
	{

class PerforceChangeList;
class PerforceChangeListFile;

/*! \brief P4 client.
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

	bool whereIsLocalFile(const std::wstring& depotFile, std::wstring& outLocalPath);

	bool isOpened(const std::wstring& localFile, PerforceAction& outAction);

	bool addFile(const PerforceChangeList* changeList, const std::wstring& localFile);

	bool openForEdit(const PerforceChangeList* changeList, const std::wstring& localFile);

	bool openForDelete(const PerforceChangeList* changeList, const std::wstring& localFile);

	bool revertFile(const PerforceChangeList* changeList, const std::wstring& localFile);

	bool synchronize();

	const std::wstring& getLastError() const;

private:
	PerforceClientDesc m_clientDesc;
	AutoPtr< ClientApi > m_p4client;
	std::wstring m_lastError;

	bool establishConnection();
};

	}
}

#endif	// traktor_db_PerforceClient_H
