#ifndef traktor_db_LocalInstance_H
#define traktor_db_LocalInstance_H

#include "Database/Provider.h"
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
	namespace db
	{

class Context;
class LocalGroup;
class Transaction;

/*! \brief Local instance.
 * \ingroup Database
 */
class T_DLLCLASS LocalInstance : public IProviderInstance
{
	T_RTTI_CLASS(LocalInstance)

public:
	LocalInstance(Context* context);

	bool internalCreate(const Path& instancePath);

	bool internalCreateNew(const Path& instancePath, const Guid& instanceGuid);

	virtual std::wstring getPrimaryTypeName() const;

	virtual bool beginTransaction();

	virtual bool endTransaction(bool commit);

	virtual std::wstring getName() const;

	virtual bool setName(const std::wstring& name);

	virtual Guid getGuid() const;

	virtual bool setGuid(const Guid& guid);

	virtual bool remove();

	virtual Serializable* getObject();

	virtual bool setObject(const Serializable* object);

	virtual uint32_t getDataNames(std::vector< std::wstring >& outDataNames) const;

	virtual Stream* readData(const std::wstring& dataName);

	virtual Stream* writeData(const std::wstring& dataName);

private:
	Ref< Context > m_context;
	Path m_instancePath;
	Ref< Transaction > m_transaction;
	std::wstring m_transactionName;
};

	}
}

#endif	// traktor_db_LocalInstance_H
