#ifndef traktor_db_FileDataAccess_H
#define traktor_db_FileDataAccess_H

#include "Database/Local/DataAccess.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

/*! \brief Physical file data access.
 * \ingroup Database
 */
class T_DLLCLASS FileDataAccess : public DataAccess
{
	T_RTTI_CLASS(FileDataAccess)

public:
	FileDataAccess(bool binary);

	virtual bool renameGroup(const Path& groupPath, const std::wstring& newGroupName) const;

	virtual bool removeGroup(const Path& groupPath) const;

	virtual bool createGroup(const Path& groupPath, const std::wstring& newGroupName) const;

	virtual bool enumerateGroups(const Path& groupPath, std::vector< Path >& outGroups) const;

	virtual bool enumerateInstances(const Path& groupPath, std::vector< Path >& outInstances) const;

	virtual Ref< LocalInstanceMeta > getInstanceMeta(const Path& instancePath) const;

	virtual bool createInstance(const Path& instancePath, const Guid& instanceGuid) const;

	virtual bool renameInstance(const Path& instancePath, const std::wstring& newInstanceName) const;

	virtual bool removeInstance(const Path& instancePath) const;

	virtual Ref< Serializable > readObject(const Path& instancePath) const;

	virtual bool writeObject(const Path& instancePath, Serializable* object) const;

	virtual uint32_t enumerateDataNames(const Path& instancePath, std::vector< std::wstring >& outDataNames) const;

	virtual Ref< Stream > readData(const Path& instancePath, const std::wstring& dataName) const;

	virtual Ref< Stream > writeData(const Path& instancePath, const std::wstring& dataName) const;

private:
	bool m_binary;
};

	}
}

#endif	// traktor_db_FileDataAccess_H
