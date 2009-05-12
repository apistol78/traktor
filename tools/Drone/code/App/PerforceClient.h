#ifndef traktor_drone_PerforceClient_H
#define traktor_drone_PerforceClient_H

#include <vector>
#include <clientapi.h>
#include <Core/Heap/Ref.h>
#include <Core/Object.h>
#include <Core/Io/Path.h>
#include "App/PerforceTypes.h"
#include "App/PerforceClientDesc.h"

namespace traktor
{
	namespace drone
	{

class PerforceChangeList;
class PerforceChangeListFile;

class PerforceClient : public Object
{
	T_RTTI_CLASS(PerforceClient)

public:
	PerforceClient(const PerforceClientDesc& clientDesc);

	virtual ~PerforceClient();

	bool getChangeLists(RefArray< PerforceChangeList >& outChangeLists);

	PerforceChangeList* createChangeList(const std::wstring& description);

	bool whereIsLocalFile(const std::wstring& depotFile, std::wstring& outLocalPath);

	bool isOpened(const std::wstring& localFile, PerforceAction& outAction);

	bool addFile(const PerforceChangeList* changeList, const std::wstring& localFile);

	bool openForEdit(const PerforceChangeList* changeList, const std::wstring& localFile);

	bool openForDelete(const PerforceChangeList* changeList, const std::wstring& localFile);

	bool synchronize();

	const std::wstring& getLastError() const;

private:
	PerforceClientDesc m_clientDesc;
	ClientApi m_p4client;
	bool m_connected;
	std::wstring m_lastError;

	bool establishConnection();
};

	}
}

#endif	// traktor_drone_PerforceClient_H
