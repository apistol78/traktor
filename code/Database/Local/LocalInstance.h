/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_LocalInstance_H
#define traktor_db_LocalInstance_H

#include "Database/Provider/IProviderInstance.h"
#include "Core/Io/Path.h"

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
class LocalInstance : public IProviderInstance
{
	T_RTTI_CLASS;

public:
	LocalInstance(Context* context, const Path& instancePath);

	bool internalCreateNew(const Guid& instanceGuid);

	virtual std::wstring getPrimaryTypeName() const T_OVERRIDE T_FINAL;

	virtual bool openTransaction() T_OVERRIDE T_FINAL;

	virtual bool commitTransaction() T_OVERRIDE T_FINAL;

	virtual bool closeTransaction() T_OVERRIDE T_FINAL;

	virtual std::wstring getName() const T_OVERRIDE T_FINAL;

	virtual bool setName(const std::wstring& name) T_OVERRIDE T_FINAL;

	virtual Guid getGuid() const T_OVERRIDE T_FINAL;

	virtual bool setGuid(const Guid& guid) T_OVERRIDE T_FINAL;

	virtual bool getLastModifyDate(DateTime& outModifyDate) const T_OVERRIDE T_FINAL;

	virtual uint32_t getFlags() const T_OVERRIDE T_FINAL;

	virtual bool remove() T_OVERRIDE T_FINAL;

	virtual Ref< IStream > readObject(const TypeInfo*& outSerializerType) const T_OVERRIDE T_FINAL;

	virtual Ref< IStream > writeObject(const std::wstring& primaryTypeName, const TypeInfo*& outSerializerType) T_OVERRIDE T_FINAL;

	virtual uint32_t getDataNames(std::vector< std::wstring >& outDataNames) const T_OVERRIDE T_FINAL;

	virtual bool getDataLastWriteTime(const std::wstring& dataName, DateTime& outLastWriteTime) const T_OVERRIDE T_FINAL;

	virtual bool removeAllData() T_OVERRIDE T_FINAL;

	virtual Ref< IStream > readData(const std::wstring& dataName) const T_OVERRIDE T_FINAL;

	virtual Ref< IStream > writeData(const std::wstring& dataName) T_OVERRIDE T_FINAL;

private:
	Ref< Context > m_context;
	Path m_instancePath;
	Ref< Transaction > m_transaction;
	std::wstring m_transactionName;
};

	}
}

#endif	// traktor_db_LocalInstance_H
