#ifndef traktor_drone_PerforceChangeListFile_H
#define traktor_drone_PerforceChangeListFile_H

#include <Core/Serialization/Serializable.h>
#include "App/PerforceTypes.h"

namespace traktor
{
	namespace drone
	{

class PerforceChangeListFile : public Serializable
{
	T_RTTI_CLASS(PerforceChangeListFile)

public:
	PerforceChangeListFile();

	void setDepotPath(const std::wstring& depotPath);

	const std::wstring& getDepotPath() const;

	void setLocalPath(const std::wstring& localPath);

	const std::wstring& getLocalPath() const;

	void setAction(PerforceAction action);

	PerforceAction getAction() const;

	virtual bool serialize(Serializer& s);

private:
	std::wstring m_depotPath;
	std::wstring m_localPath;
	PerforceAction m_action;
};

	}
}

#endif	// traktor_drone_PerforceChangeListFile_H
