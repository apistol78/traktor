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
#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Date/DateTime.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;
class IStream;

	namespace db
	{

/*! Provider instance interface.
 * \ingroup Database
 */
class T_DLLCLASS IProviderInstance : public Object
{
	T_RTTI_CLASS;

public:
	/*! Get primary type of instance.
	 *
	 * \return Type name of instance.
	 */
	virtual std::wstring getPrimaryTypeName() const = 0;

	/*! Open modification transaction of instance.
	 *
	 * \return True if transaction opened.
	 */
	virtual bool openTransaction() = 0;

	/* Commit changes performed within transaction.
	 *
	 * \return True if all changed committed successfully.
	 */
	virtual bool commitTransaction() = 0;

	/*! Close transaction of instance, all non-committed changes is discarded.
	 *
	 * \return True if transaction was closed.
	 */
	virtual bool closeTransaction() = 0;

	/*! Get name of instance.
	 *
	 * \return Name of instance.
	 */
	virtual std::wstring getName() const = 0;

	/*! Set name of instance.
	 *
	 * \note
	 * Must be performed within a transaction.
	 *
	 * \return True if instance's name was changed.
	 */
	virtual bool setName(const std::wstring& name) = 0;

	/*! Get guid of instance.
	 *
	 * \return Guid of instance.
	 */
	virtual Guid getGuid() const = 0;

	/*! Set guid of instance.
	 *
	 * \note
	 * Must be performed within a transaction.
	 *
	 * \param guid New guid of instance.
	 * \return True if instance's guid was changed.
	 */
	virtual bool setGuid(const Guid& guid) = 0;

	/*! Get last date this instance was changed.
	 *
	 * \param outModifyDate Modification date.
	 * \return True if modification date was retrieved.
	 */
	virtual bool getLastModifyDate(DateTime& outModifyDate) const = 0;

	/*! Get flags from instance storage.
	 *
	 * \param Instance storage flags.
	 */
	virtual uint32_t getFlags() const = 0;

	/*! Remove instance.
	 *
	 * \note
	 * Must be performed within a transaction.
	 *
	 * \return True if instance was removed.
	 */
	virtual bool remove() = 0;

	/*! Read object contained by instance.
	 *
	 * \param outSerializerType Serializer type which must be used to read object from stream.
	 * \return Stream to serialized object.
	 */
	virtual Ref< IStream > readObject(const TypeInfo*& outSerializerType) const = 0;

	/*! Write object to instance.
	 *
	 * \note
	 * Must be performed within a transaction.
	 *
	 * \param primaryTypeName Type name of object.
	 * \param outSerializerType Serializer type which must be used to write object into stream.
	 * \return Stream to serialize object into.
	 */
	virtual Ref< IStream > writeObject(const std::wstring& primaryTypeName, const TypeInfo*& outSerializerType) = 0;

	/*! Get list of names to data blobs contained within instance.
	 *
	 * \param outDataNames List of names of data blobs.
	 * \return Number of data names contained within instance.
	 */
	virtual uint32_t getDataNames(std::vector< std::wstring >& outDataNames) const = 0;

	/*! Get date when a data blob was last modified.
	 *
	 * \param dataName Name of data blob.
	 * \param outLastWriteTime Last modification date of data blob.
	 * \return True if date was received.
	 */
	virtual bool getDataLastWriteTime(const std::wstring& dataName, DateTime& outLastWriteTime) const = 0;

	/*! Remove all data blobs from instance.
	 *
	 * \note
	 * Must be performed within a transaction.
	 *
	 * \return True if all data blobs was removed.
	 */
	virtual bool removeAllData() = 0;

	/*! Read data blob contained within instance.
	 *
	 * \param dataName Name of data blob.
	 * \return Stream to data blob.
	 */
	virtual Ref< IStream > readData(const std::wstring& dataName) const = 0;

	/*! Write data to blob contained within instance.
	 *
	 * \note
	 * Must be performed within a transaction.
	 *
	 * \param dataName Name of data blob.
	 * \return Stream to data blob.
	 */
	virtual Ref< IStream > writeData(const std::wstring& dataName) = 0;
};

	}
}

