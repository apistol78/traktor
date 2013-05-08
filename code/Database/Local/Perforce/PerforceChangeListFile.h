#ifndef traktor_db_PerforceChangeListFile_H
#define traktor_db_PerforceChangeListFile_H

#include "Core/Serialization/ISerializable.h"
#include "Database/Local/Perforce/PerforceTypes.h"

namespace traktor
{
	namespace db
	{

/*! \brief P4 change list file.
 * \ingroup Database
 */
class PerforceChangeListFile : public ISerializable
{
	T_RTTI_CLASS;

public:
	PerforceChangeListFile();

	void setDepotPath(const std::wstring& depotPath);

	const std::wstring& getDepotPath() const;

	void setLocalPath(const std::wstring& localPath);

	const std::wstring& getLocalPath() const;

	void setAction(PerforceAction action);

	PerforceAction getAction() const;

	virtual void serialize(ISerializer& s);

private:
	std::wstring m_depotPath;
	std::wstring m_localPath;
	PerforceAction m_action;
};

	}
}

#endif	// traktor_db_PerforceChangeListFile_H
