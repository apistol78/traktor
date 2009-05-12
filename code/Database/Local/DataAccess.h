#ifndef traktor_db_DataAccess_H
#define traktor_db_DataAccess_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Io/Path.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Guid;
class Stream;
class Serializable;

	namespace db
	{

class LocalInstanceMeta;

/*! \brief Physical data accessors.
 * \ingroup Database
 *
 * All access to physical data stored in local database
 * is through this interface.
 */
class T_DLLCLASS DataAccess : public Object
{
	T_RTTI_CLASS(DataAccess)

public:
	/*! \name Group data accessors. */
	//@{

	virtual bool renameGroup(const Path& groupPath, const std::wstring& newGroupName) const = 0;

	virtual bool removeGroup(const Path& groupPath) const = 0;

	virtual bool createGroup(const Path& groupPath, const std::wstring& newGroupName) const = 0;

	virtual bool enumerateGroups(const Path& groupPath, std::vector< Path >& outGroups) const = 0;

	virtual bool enumerateInstances(const Path& groupPath, std::vector< Path >& outInstances) const = 0;

	//@}

	/*! \name Instance data accessors. */
	//@{

	virtual Ref< LocalInstanceMeta > getInstanceMeta(const Path& instancePath) const = 0;

	virtual bool createInstance(const Path& instancePath, const Guid& instanceGuid) const = 0;

	virtual bool renameInstance(const Path& instancePath, const std::wstring& newInstanceName) const = 0;

	virtual bool removeInstance(const Path& instancePath) const = 0;

	virtual Ref< Serializable > readObject(const Path& instancePath) const = 0;

	virtual bool writeObject(const Path& instancePath, Serializable* object) const = 0;

	virtual uint32_t enumerateDataNames(const Path& instancePath, std::vector< std::wstring >& outDataNames) const = 0;

	virtual Ref< Stream > readData(const Path& instancePath, const std::wstring& dataName) const = 0;

	virtual Ref< Stream > writeData(const Path& instancePath, const std::wstring& dataName) const = 0;

	//@}
};

	}
}

#endif	// traktor_db_DataAccess_H
