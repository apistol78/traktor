/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Io/Path.h"
#include "Database/Provider/IProviderInstance.h"

namespace traktor
{
	namespace db
	{

class Context;
class LocalGroup;
class Transaction;

/*! Local instance.
 * \ingroup Database
 */
class LocalInstance : public IProviderInstance
{
	T_RTTI_CLASS;

public:
	explicit LocalInstance(Context& context, const Path& instancePath);

	bool internalCreateNew(const Guid& instanceGuid);

	virtual std::wstring getPrimaryTypeName() const override final;

	virtual bool openTransaction() override final;

	virtual bool commitTransaction() override final;

	virtual bool closeTransaction() override final;

	virtual std::wstring getName() const override final;

	virtual bool setName(const std::wstring& name) override final;

	virtual Guid getGuid() const override final;

	virtual bool setGuid(const Guid& guid) override final;

	virtual bool getLastModifyDate(DateTime& outModifyDate) const override final;

	virtual uint32_t getFlags() const override final;

	virtual bool remove() override final;

	virtual Ref< IStream > readObject(const TypeInfo*& outSerializerType) const override final;

	virtual Ref< IStream > writeObject(const std::wstring& primaryTypeName, const TypeInfo*& outSerializerType) override final;

	virtual uint32_t getDataNames(std::vector< std::wstring >& outDataNames) const override final;

	virtual bool getDataLastWriteTime(const std::wstring& dataName, DateTime& outLastWriteTime) const override final;

	virtual bool removeAllData() override final;

	virtual Ref< IStream > readData(const std::wstring& dataName) const override final;

	virtual Ref< IStream > writeData(const std::wstring& dataName) override final;

private:
	Context& m_context;
	Path m_instancePath;
	Ref< Transaction > m_transaction;
	std::wstring m_transactionName;
};

	}
}

